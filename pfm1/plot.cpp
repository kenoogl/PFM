#include <stdio.h>
#include <stdlib.h>

#include "wingxa.h"

#define ND 100
#define INX 400
#define INY 400

namespace {

const char kDefaultInputFile[] = "output/test.dat";

int nd = ND;
int ndm = ND - 1;
double time1;
double c2h[ND][ND];
double c3h[ND][ND];

void graph_a();

}  // namespace

int main(int argc, char **argv)
{
	int i, j;
	const char *fname = argc >= 2 ? argv[1] : kDefaultInputFile;
	FILE *datin0 = fopen(fname, "r");

	if (datin0 == NULL) {
		perror(fname);
		return EXIT_FAILURE;
	}

	gwinsize(INX, INY);
	ginit(2);
	gsetorg(0, 0);

start:

	if (fscanf(datin0, "%lf", &time1) != 1) {
		fclose(datin0);
		return EXIT_SUCCESS;
	}
	for (i = 0; i <= ndm; i++) {
		for (j = 0; j <= ndm; j++) {
			if (fscanf(datin0, "%lf %lf", &c2h[i][j], &c3h[i][j]) != 2) {
				fprintf(stderr, "unexpected end of file while reading %s\n", fname);
				fclose(datin0);
				return EXIT_FAILURE;
			}
		}
	}

	graph_a();

	if (keypress()) {
		fclose(datin0);
		return EXIT_SUCCESS;
	}

	if (feof(datin0) == 0) {
		goto start;
	}
	fclose(datin0);

	return EXIT_SUCCESS;
}

namespace {

void graph_a()
{
	int i, j, ii, jj;
	double col_R, col_G, col_B;
	double x, xmax, xmin, y, ymax, ymin, rad0;
	int ixmin = 0, iymin = 0, igx, igy, irad0;
	int ixmax = INX, iymax = INY;

	xmin = 0.0;
	xmax = 1.0;
	ymin = 0.0;
	ymax = 1.0;
	printf("time %f\n", time1);
	rad0 = 1.0 / (double)nd / 2.0;
	irad0 = (int)((ixmax - ixmin) / (xmax - xmin) * rad0 + 1);

	for (i = 0; i <= nd; i++) {
		for (j = 0; j <= nd; j++) {
			x = 1.0 / (double)nd * i + rad0;
			igx = (int)((ixmax - ixmin) / (xmax - xmin) * (x - xmin) + ixmin);
			y = 1.0 / (double)nd * j + rad0;
			igy = (int)((iymax - iymin) / (ymax - ymin) * (y - ymin) + iymin);

			ii = i;
			jj = j;
			if (i == nd) {
				ii = 0;
			}
			if (j == nd) {
				jj = 0;
			}

			col_G = c2h[ii][jj];
			col_B = c3h[ii][jj];
			col_R = 1.0 - c2h[ii][jj] - c3h[ii][jj];
			if (col_R > 1.0) {
				col_R = 1.0;
			}
			if (col_R < 0.0) {
				col_R = 0.0;
			}
			if (col_G > 1.0) {
				col_G = 1.0;
			}
			if (col_G < 0.0) {
				col_G = 0.0;
			}
			if (col_B > 1.0) {
				col_B = 1.0;
			}
			if (col_B < 0.0) {
				col_B = 0.0;
			}

			gcolor((int)(255 * col_R), (int)(255 * col_G), (int)(255 * col_B));
			grect(igx - irad0, igy - irad0, igx + irad0, igy + irad0);
		}
	}
	swapbuffers();
}

}  // namespace
