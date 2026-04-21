#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <math.h>
#include <string.h>
#include "wingxa.h"

#define ND 100 //差分分割数
#define INX 400 //windowのxサイズ（ピクセル）
#define INY 400 //windowのyサイズ（ピクセル）

	int nd=ND, ndm=ND-1, nd2=ND/2;//差分分割数、差分分割数-1、差分分割数/2
	double PI=3.141592654, time1;//π、計算ステップ
	double c2h[ND][ND], c3h[ND][ND];//場の配列データ

	void graph_a();			//場の描画サブル－チン

//******* メインプログラム ****************************************************
int main(void)
{
 	gwinsize(INX,INY); ginit(1); gsetorg(0,0);//window設定

	int   	i, j;
	char	fname[100];
	FILE	*datin0;

	int ii=1;//bmp_n[i]のiの初期値
	char bmp_file0[100], bmp_file1[100], bmp_file2[100];//画像ファイル名の配列
	int bmp_n[100];//保存する画像の時間ステップ

//##### 計算結果のデータファイル名、画像ファイル名、保存する時間ステップの記入 ###############
	strcpy(fname, "test1.dat");			//計算結果のデータが保存されているファイル名を設定
	strcpy(bmp_file1, "c_field_");
    //画像ファイル名の一部（これに以下の時間ステップが追加され、保存する画像ファイル名となる）

	bmp_n[1]=0;
	bmp_n[2]=2000;
	bmp_n[3]=4000;
	bmp_n[4]=6000;
	bmp_n[5]=8000;
	bmp_n[6]=10000;
	bmp_n[7]=12000;
	bmp_n[8]=14000;
	bmp_n[9]=16000;
	bmp_n[10]=18000;
	bmp_n[11]=20000;
	bmp_n[12]=30000;
	bmp_n[13]=40000;
	bmp_n[14]=50000;
	bmp_n[15]=60000;
	bmp_n[16]=70000;
	bmp_n[17]=80000;

//#########################################################################

	strcpy(bmp_file0, bmp_file1);		//bmp_file1をbmp_file0にコピー

	datin0 = fopen(fname, "r");			//デ－タファイルを開く

start: ;

	fscanf(datin0, "%lf", &time1);	//計算ステップを読む
	for(i=0;i<=ndm;i++){
		for(j=0;j<=ndm;j++){
			fscanf(datin0, "%lf %lf ", &c2h[i][j], &c3h[i][j]);//場のデータを読む
		}
	}

	if((int)time1==bmp_n[ii]){
		graph_a();												//画像の描画
		itoa(bmp_n[ii], bmp_file2, 10);		//bmp_n[ii]を文字列化してbmp_file2にコピー
		strcat(bmp_file1,bmp_file2);			//bmp_file1にbmp_file2を結合
		strcat(bmp_file1,".bmp");					//bmp_file1に".bmp"を結合
		save_screen(bmp_file1);						//画像をbmpファイルとして保存（ファイル名は、bmp_file1）
		ii++;															//iiを１進める
		strcpy(bmp_file1, bmp_file0);			//bmp_file1にbmp_file0をコピー

		if(keypress()){return 0;}					//キー入力の確認

	}

	if (feof(datin0)==0) {goto start;}	//データが最後でなければstartへ戻る。
   fclose(datin0);										//デ－タファイルを閉じる

  return EXIT_SUCCESS;

end:;
}

//******* 組織の描画 **************************************************
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

