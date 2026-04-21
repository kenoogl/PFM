#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <math.h>
#include "wingxa.h"	//グラフィック関連部品のヘッダファイル

#define DRND(x) ((double)(x)/RAND_MAX*rand())  //乱数の設定

#define ND 100					//差分計算における計算領域一辺の分割数
#define INX 400					//描画window１辺(x方向)のピクセルサイズ
#define INY 400					//描画window１辺(y方向)のピクセルサイズ

	int nd=ND;						//計算領域の一辺の差分分割数(差分ブロック数)
	int ndm=ND-1;					//ND-1を定義
	double PI=3.141592654, rr=8.3145, temp, time1;//π,ガス定数,絶対温度,計算カウント数(時間に比例)
	double c2a, c3a;			//平均組成(1:A成分, 2:B成分, 3:C成分)
	double c2h[ND][ND], c3h[ND][ND];//局所組成

	void ini000();		//初期濃度プロファイルの設定サブルーチン
	void graph_a();		//グラフ描画サブルーチン
	void datsave();		//データ保存サブルーチン

//******* メインプログラム ******************************************
int main(void)
{
	double c1, c2, c3;												//局所濃度
	double c2h2[ND][ND], c3h2[ND][ND];				//局所濃度場の補助行列
	double c2k_chem, c2k_su, c2k[ND][ND], c3k_chem, c3k_su, c3k[ND][ND];//局所ポテンシャル
	double dc2a, sumc2, dc3a, sumc3, sumc23;	//濃度場の収支計算に使用している変数
	double dakd2, dakd3;											//拡散ポテンシャルの二階微分
	double c2ddtt, c3ddtt;										//濃度場の時間変動量

	int   i, j;																//整数
	int   ip, im, jp, jm;											//(i+1),(i-1),(j+1),(j-1)
	double al, b1, rtemp, delt;					//計算領域一辺の長さ、差分プロック１辺の長さ、RT、時間きざみ
	double time1max;													//計算カウント数の最大値（計算終了カウント）
	double cmob22, cmob33, cmob23, cmob32;		//易動度

	double om_12, om_23, om_13;								//相互作用パラメータ
	double kapa_c2, kapa_c3;									//濃度勾配エネルギー係数

//****** 計算条件および物質定数の設定 ****************************************

	printf("C2B(0.333) =  ");	scanf(" %lf",&c2a);//標準入出力から平均組成(B成分)を入力
//	c2a=1./3.;

	printf("C3C(0.333) =  ");	scanf(" %lf",&c3a);//標準入出力から平均組成(C成分)を入力
//	c3a=1./3.;

	printf("delt(0.005)=  ");	scanf(" %lf",&delt);//時間刻み
//	delt=0.005;

	temp=900.0;					//温度（K）
	rtemp=rr*temp;			//RT
	al=100.0*1.0E-09;		//計算領域の一辺の長さ(m)
	b1=al/(double)ND;		//差分プロック１辺の長さ

	cmob22=1.0;  cmob33=1.0;  cmob23=cmob32=-0.5;//易動度

	om_12=25000./rtemp; 									//相互作用パラメータ(J/molで、RTで無次元化)
	om_23=25000./rtemp;
	om_13=25000./rtemp;

	kapa_c2=kapa_c3=5.0e-15/b1/b1/rtemp;	//濃度勾配エネルギー係数(Jm^2/molで、RTとb1^2で無次元化)

	time1=0.0;  time1max=1.0e05+1.0;			//計算カウント数の初期値と最大値

//*** 初期濃度場の設定と描画Window表示 *****************************************

	ini000();//初期濃度場の設定
 	gwinsize(INX,INY); ginit(2); gsetorg(0,0);//描画Window表示

//**** シミュレーションスタート ******************************
start: ;

	if((((int)(time1) % 100)==0)) {graph_a();} 	//一定繰返しカウント毎に濃度場を表示
	if((((int)(time1) % 2000)==0)) {datsave();} //一定繰返しカウント毎に濃度場を保存

//***** ポテンシャル場の計算 ***********************************
	for(i=0;i<=ndm;i++){
		for(j=0;j<=ndm;j++){
			ip=i+1; im=i-1; jp=j+1; jm=j-1;
			if(i==ndm){ip=0;}  if(i==0){im=ndm;}//周期的境界条件
			if(j==ndm){jp=0;}  if(j==0){jm=ndm;}

			c2=c2h[i][j];  c3=c3h[i][j];  c1=1.0-c2-c3;//局所濃度場

			c2k_chem=om_12*(c1-c2)-om_13*c3+om_23*c3+(log(c2)-log(c1));//化学拡散ポテンシャル
			c2k_su=-2.*kapa_c2*(c2h[ip][j]+c2h[im][j]+c2h[i][jp]+c2h[i][jm]-4.0*c2)
							  -kapa_c3*(c3h[ip][j]+c3h[im][j]+c3h[i][jp]+c3h[i][jm]-4.0*c3);//勾配ポテンシャル

			c3k_chem=om_13*(c1-c3)-om_12*c2+om_23*c2+(log(c3)-log(c1));//化学拡散ポテンシャル
			c3k_su=-2.*kapa_c3*(c3h[ip][j]+c3h[im][j]+c3h[i][jp]+c3h[i][jm]-4.0*c3)
							  -kapa_c2*(c2h[ip][j]+c2h[im][j]+c2h[i][jp]+c2h[i][jm]-4.0*c2);//勾配ポテンシャル

			c2k[i][j]=c2k_chem+c2k_su;//拡散ポテンシャル(式(4.1))
			c3k[i][j]=c3k_chem+c3k_su;
		}
	}

//***** 発展方程式の計算 **********************************
	for(i=0;i<=ndm;i++){
		for(j=0;j<=ndm;j++){
			ip=i+1; im=i-1; jp=j+1; jm=j-1;
			if(i==ndm){ip=0;} if(i==0){im=ndm;}//周期的境界条件
			if(j==ndm){jp=0;} if(j==0){jm=ndm;}

			dakd2=c2k[ip][j]+c2k[im][j]+c2k[i][jp]+c2k[i][jm]-4.0*c2k[i][j];//拡散ポテンシャルの二階微分
			dakd3=c3k[ip][j]+c3k[im][j]+c3k[i][jp]+c3k[i][jm]-4.0*c3k[i][j];

			c2ddtt=cmob22*dakd2+cmob23*dakd3;//拡散方程式(式(4.2))
			c3ddtt=cmob32*dakd2+cmob33*dakd3;

			c2h2[i][j]=c2h[i][j]+c2ddtt*delt;//濃度場の時間発展
			c3h2[i][j]=c3h[i][j]+c3ddtt*delt;

			if(c2h[i][j]>=1.0){c2h[i][j]=1.0-1.0e-06;}//濃度場の変域補正
			if(c2h[i][j]<=0.0){c2h[i][j]=1.0e-06;}
			if(c3h[i][j]>=1.0){c3h[i][j]=1.0-1.0e-06;}
			if(c3h[i][j]<=0.0){c3h[i][j]=1.0e-06;}
		}
	}

//*** 濃度場の収支補正 ***********************************************
	sumc2=0.; for(i=0;i<=ndm;i++){  for(j=0;j<=ndm;j++){ sumc2+=c2h2[i][j]; } }
  dc2a=sumc2/ND/ND-c2a;
	for(i=0;i<=ndm;i++){  for(j=0;j<=ndm;j++){ c2h[i][j]=c2h2[i][j]-dc2a; } }

	sumc3=0.; for(i=0;i<=ndm;i++){  for(j=0;j<=ndm;j++){ sumc3+=c3h2[i][j]; } }
	dc3a=sumc3/ND/ND-c3a;
	for(i=0;i<=ndm;i++){  for(j=0;j<=ndm;j++){ c3h[i][j]=c3h2[i][j]-dc3a; } }

	for(i=0;i<=ndm;i++){
    for(j=0;j<=ndm;j++){
			sumc23=c2h[i][j]+c3h[i][j];
			if(sumc23>=1.0){
				c2h[i][j]=c2h[i][j]/sumc23-1.0e-06;
				c3h[i][j]=c3h[i][j]/sumc23-1.0e-06;
			}
    }
	}
//*********************************************************************

	if(keypress()){return 0;}	//キー待ち状態
	time1=time1+1.0;  if(time1<time1max){goto start;}//最大カウント数に到達したかどうかの判断

	end:;
  return 0;
}


//************ 初期濃度場の設定サブルーチン *************
void ini000()
{
	int i, j, id;
 	//srand(time(NULL));//乱数の種設定

	for(i=0;i<=ndm;i++){
		for(j=0;j<=ndm;j++){
			c2h[i][j]=c2a+0.01*(2.0*DRND(1)-1.0);//濃度場を最大±1%の乱数にて設定
			c3h[i][j]=c3a+0.01*(2.0*DRND(1)-1.0);
		}
	}

}

//******* 濃度場の描画サブルーチン ***************************************
void graph_a()
{
	int i, j, ii, jj;														//整数
	double col_R, col_G, col_B;									//色
	double x, xmax, xmin, y, ymax, ymin, rad0;	//規格化座標系の設定
	int ixmin=0, iymin=0, igx, igy, irad0;			//スクリーン座標系の設定
	int ixmax=INX, iymax=INY;										//描画Window範囲

	//gcls(); //画面クリア
	xmin=0.0; xmax=1.0; ymin=0.0; ymax=1.0;			//描画領域（規格化されている）
	printf("time %f\n",time1);									//計算カウント数の表示
	rad0=1.0/(double)nd/2.0;    irad0=(ixmax-ixmin)/(xmax-xmin)*rad0+1;
	//差分ブロックの半分の長さ	//スクリーン座標系に変換（+1は整数化時の切捨て補正）

	for(i=0;i<=nd;i++){
		for(j=0;j<=nd;j++){
			x=1.0/(double)nd*i+rad0;  igx=(ixmax-ixmin)/(xmax-xmin)*(x-xmin)+ixmin;
			y=1.0/(double)nd*j+rad0;  igy=(iymax-iymin)/(ymax-ymin)*(y-ymin)+iymin;
			//座標計算								//スクリーン座標系に変換
			ii=i; jj=j;  if(i==nd){ii=0;}  if(j==nd){jj=0;}//周期的境界条件

			col_G=c2h[ii][jj];  col_B=c3h[ii][jj];  col_R=1.0-c2h[ii][jj]-c3h[ii][jj];//濃度場をRGBに設定
			if(col_R>1.0){col_R=1.0;}  if(col_R<0.0){col_R=0.0;}//RGBの変域補正
			if(col_G>1.0){col_G=1.0;}  if(col_G<0.0){col_G=0.0;}
			if(col_B>1.0){col_B=1.0;}  if(col_B<0.0){col_B=0.0;}

			gcolor((int)(255*col_R),(int)(255*col_G),(int)(255*col_B));//色設定
			grect(igx-irad0,igy-irad0,igx+irad0,igy+irad0);//中塗り四角形の描画

		}
	}
	swapbuffers();//画面スワップ
}

//************ データ保存サブルーチン *******************************
void datsave()
{
	FILE *stream;	//ストリームのポインタ設定
	int i, j;			//整数

	stream = fopen("test.dat", "a");	//書き込む先のファイルを追記方式でオープン
	fprintf(stream, "%f\n", time1);		//計算カウント数の保存
	for(i=0;i<=ndm;i++){
		for(j=0;j<=ndm;j++){
			fprintf(stream, "%e  %e  ", c2h[i][j], c3h[i][j]);//局所濃度場の保存
		}
	}
	fprintf(stream, "\n");	//改行の書き込み
	fclose(stream);					//ファイルをクローズ
}

