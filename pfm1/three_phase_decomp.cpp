#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "wingxa.h"

#define DRND(x) ((double)(x) / RAND_MAX * rand())

#define ND 100
#define INX 400
#define INY 400

namespace {

const int kDefaultDataSaveInterval = 2000;
const int kDefaultBmpSaveInterval = 2000;
const int kDefaultMaxStep = 100000;
const char kDefaultOutputDir[] = "output";
const char kBmpPrefix[] = "c_field_";

int nd = ND;
int ndm = ND - 1;
double rr = 8.3145;
double temp;
double time1;
double c2a;
double c3a;
double c2h[ND][ND];
double c3h[ND][ND];
int g_data_save_interval = kDefaultDataSaveInterval;
int g_bmp_save_interval = kDefaultBmpSaveInterval;
int g_max_step = kDefaultMaxStep;
char g_output_dir[256] = "output";
char g_data_file_path[512] = "";

void ini000();
void graph_a();
void datsave(const char *filename);
void save_bitmap_snapshot(int step);
void clamp_concentrations(double &c2, double &c3);
void configure_paths();
void ensure_output_dir();
void print_usage(const char *program_name);
void parse_args(int argc, char **argv, double *delt);
int parse_int_arg(const char *name, const char *value);
double parse_double_arg(const char *name, const char *value);

}  // namespace

int main(int argc, char **argv)
{
	double c1, c2, c3;
	double c2h2[ND][ND], c3h2[ND][ND];
	double c2k_chem, c2k_su, c2k[ND][ND], c3k_chem, c3k_su, c3k[ND][ND];
	double dc2a, sumc2, dc3a, sumc3, sumc23;
	double dakd2, dakd3;
	double c2ddtt, c3ddtt;

	int i, j;
	int ip, im, jp, jm;
	double al, b1, rtemp, delt;
	double time1max;
	double cmob22, cmob33, cmob23, cmob32;

	double om_12, om_23, om_13;
	double kapa_c2, kapa_c3;

	parse_args(argc, argv, &delt);

	temp = 900.0;
	rtemp = rr * temp;
	al = 100.0 * 1.0E-09;
	b1 = al / (double)ND;

	cmob22 = 1.0;
	cmob33 = 1.0;
	cmob23 = cmob32 = -0.5;

	om_12 = 25000.0 / rtemp;
	om_23 = 25000.0 / rtemp;
	om_13 = 25000.0 / rtemp;

	kapa_c2 = kapa_c3 = 5.0e-15 / b1 / b1 / rtemp;

	time1 = 0.0;
	time1max = g_max_step + 1.0;

	configure_paths();
	ensure_output_dir();

	ini000();
	gwinsize(INX, INY);
	ginit(2);
	gsetorg(0, 0);

	graph_a();
	datsave(g_data_file_path);
	save_bitmap_snapshot((int)time1);

start:

	for (i = 0; i <= ndm; i++) {
		for (j = 0; j <= ndm; j++) {
			ip = i + 1;
			im = i - 1;
			jp = j + 1;
			jm = j - 1;
			if (i == ndm) {
				ip = 0;
			}
			if (i == 0) {
				im = ndm;
			}
			if (j == ndm) {
				jp = 0;
			}
			if (j == 0) {
				jm = ndm;
			}

			c2 = c2h[i][j];
			c3 = c3h[i][j];
			clamp_concentrations(c2, c3);
			c1 = 1.0 - c2 - c3;

			c2k_chem = om_12 * (c1 - c2) - om_13 * c3 + om_23 * c3 + (log(c2) - log(c1));
			c2k_su = -2.0 * kapa_c2 * (c2h[ip][j] + c2h[im][j] + c2h[i][jp] + c2h[i][jm] - 4.0 * c2)
			          -kapa_c3 * (c3h[ip][j] + c3h[im][j] + c3h[i][jp] + c3h[i][jm] - 4.0 * c3);

			c3k_chem = om_13 * (c1 - c3) - om_12 * c2 + om_23 * c2 + (log(c3) - log(c1));
			c3k_su = -2.0 * kapa_c3 * (c3h[ip][j] + c3h[im][j] + c3h[i][jp] + c3h[i][jm] - 4.0 * c3)
			          -kapa_c2 * (c2h[ip][j] + c2h[im][j] + c2h[i][jp] + c2h[i][jm] - 4.0 * c2);

			c2k[i][j] = c2k_chem + c2k_su;
			c3k[i][j] = c3k_chem + c3k_su;
		}
	}

	for (i = 0; i <= ndm; i++) {
		for (j = 0; j <= ndm; j++) {
			ip = i + 1;
			im = i - 1;
			jp = j + 1;
			jm = j - 1;
			if (i == ndm) {
				ip = 0;
			}
			if (i == 0) {
				im = ndm;
			}
			if (j == ndm) {
				jp = 0;
			}
			if (j == 0) {
				jm = ndm;
			}

			dakd2 = c2k[ip][j] + c2k[im][j] + c2k[i][jp] + c2k[i][jm] - 4.0 * c2k[i][j];
			dakd3 = c3k[ip][j] + c3k[im][j] + c3k[i][jp] + c3k[i][jm] - 4.0 * c3k[i][j];

			c2ddtt = cmob22 * dakd2 + cmob23 * dakd3;
			c3ddtt = cmob32 * dakd2 + cmob33 * dakd3;

			c2h2[i][j] = c2h[i][j] + c2ddtt * delt;
			c3h2[i][j] = c3h[i][j] + c3ddtt * delt;
			clamp_concentrations(c2h2[i][j], c3h2[i][j]);
		}
	}

	sumc2 = 0.0;
	for (i = 0; i <= ndm; i++) {
		for (j = 0; j <= ndm; j++) {
			sumc2 += c2h2[i][j];
		}
	}
	dc2a = sumc2 / ND / ND - c2a;
	for (i = 0; i <= ndm; i++) {
		for (j = 0; j <= ndm; j++) {
			c2h[i][j] = c2h2[i][j] - dc2a;
		}
	}

	sumc3 = 0.0;
	for (i = 0; i <= ndm; i++) {
		for (j = 0; j <= ndm; j++) {
			sumc3 += c3h2[i][j];
		}
	}
	dc3a = sumc3 / ND / ND - c3a;
	for (i = 0; i <= ndm; i++) {
		for (j = 0; j <= ndm; j++) {
			c3h[i][j] = c3h2[i][j] - dc3a;
			clamp_concentrations(c2h[i][j], c3h[i][j]);
		}
	}

	for (i = 0; i <= ndm; i++) {
		for (j = 0; j <= ndm; j++) {
			sumc23 = c2h[i][j] + c3h[i][j];
			if (sumc23 >= 1.0) {
				c2h[i][j] = c2h[i][j] / sumc23 - 1.0e-06;
				c3h[i][j] = c3h[i][j] / sumc23 - 1.0e-06;
			}
			clamp_concentrations(c2h[i][j], c3h[i][j]);
		}
	}

	time1 = time1 + 1.0;
	if (((int)time1 % 100) == 0) {
		graph_a();
	}
	if (g_data_save_interval > 0 && ((int)time1 % g_data_save_interval) == 0) {
		datsave(g_data_file_path);
	}
	if (g_bmp_save_interval > 0 && ((int)time1 % g_bmp_save_interval) == 0) {
		save_bitmap_snapshot((int)time1);
	}

	if (keypress()) {
		return 0;
	}
	if (time1 < time1max) {
		goto start;
	}

	return 0;
}

namespace {

void print_usage(const char *program_name)
{
	fprintf(stderr,
	        "Usage: %s [--c2 value] [--c3 value] [--delt value] [--max-step n] "
	        "[--data-interval n] [--bmp-interval n] [--output-dir dir]\n",
	        program_name);
}

int parse_int_arg(const char *name, const char *value)
{
	char *end = NULL;
	long parsed = strtol(value, &end, 10);
	if (value[0] == '\0' || end == NULL || *end != '\0' || parsed < 0) {
		fprintf(stderr, "invalid %s: %s\n", name, value);
		exit(EXIT_FAILURE);
	}
	return (int)parsed;
}

double parse_double_arg(const char *name, const char *value)
{
	char *end = NULL;
	double parsed = strtod(value, &end);
	if (value[0] == '\0' || end == NULL || *end != '\0') {
		fprintf(stderr, "invalid %s: %s\n", name, value);
		exit(EXIT_FAILURE);
	}
	return parsed;
}

void configure_paths()
{
	snprintf(g_data_file_path, sizeof(g_data_file_path), "%s/test.dat", g_output_dir);
}

void ensure_output_dir()
{
	struct stat st;
	if (stat(g_output_dir, &st) == 0) {
		if (!S_ISDIR(st.st_mode)) {
			fprintf(stderr, "%s exists but is not a directory\n", g_output_dir);
			exit(EXIT_FAILURE);
		}
		return;
	}
	if (mkdir(g_output_dir, 0755) != 0) {
		perror("mkdir");
		exit(EXIT_FAILURE);
	}
}

void parse_args(int argc, char **argv, double *delt)
{
	bool has_c2 = false;
	bool has_c3 = false;
	bool has_delt = false;

	for (int i = 1; i < argc; ++i) {
		const char *arg = argv[i];
		if (strcmp(arg, "--help") == 0) {
			print_usage(argv[0]);
			exit(EXIT_SUCCESS);
		}
		if (i + 1 >= argc) {
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}
		const char *value = argv[++i];
		if (strcmp(arg, "--c2") == 0) {
			c2a = parse_double_arg("--c2", value);
			has_c2 = true;
		} else if (strcmp(arg, "--c3") == 0) {
			c3a = parse_double_arg("--c3", value);
			has_c3 = true;
		} else if (strcmp(arg, "--delt") == 0) {
			*delt = parse_double_arg("--delt", value);
			has_delt = true;
		} else if (strcmp(arg, "--max-step") == 0) {
			g_max_step = parse_int_arg("--max-step", value);
		} else if (strcmp(arg, "--data-interval") == 0) {
			g_data_save_interval = parse_int_arg("--data-interval", value);
		} else if (strcmp(arg, "--bmp-interval") == 0) {
			g_bmp_save_interval = parse_int_arg("--bmp-interval", value);
		} else if (strcmp(arg, "--output-dir") == 0) {
			if (strlen(value) >= sizeof(g_output_dir)) {
				fprintf(stderr, "output directory path is too long: %s\n", value);
				exit(EXIT_FAILURE);
			}
			strcpy(g_output_dir, value);
		} else {
			fprintf(stderr, "unknown option: %s\n", arg);
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (!has_c2) {
		printf("C2B(0.333) =  ");
		scanf(" %lf", &c2a);
	}
	if (!has_c3) {
		printf("C3C(0.333) =  ");
		scanf(" %lf", &c3a);
	}
	if (!has_delt) {
		printf("delt(0.005)=  ");
		scanf(" %lf", delt);
	}
}

void clamp_concentrations(double &c2, double &c3)
{
	const double kEps = 1.0e-06;

	if (c2 >= 1.0) {
		c2 = 1.0 - kEps;
	}
	if (c2 <= 0.0) {
		c2 = kEps;
	}
	if (c3 >= 1.0) {
		c3 = 1.0 - kEps;
	}
	if (c3 <= 0.0) {
		c3 = kEps;
	}
	if (c2 + c3 >= 1.0) {
		const double scale = (1.0 - 2.0 * kEps) / (c2 + c3);
		c2 *= scale;
		c3 *= scale;
	}
}

void ini000()
{
	int i, j;

	for (i = 0; i <= ndm; i++) {
		for (j = 0; j <= ndm; j++) {
			c2h[i][j] = c2a + 0.01 * (2.0 * DRND(1) - 1.0);
			c3h[i][j] = c3a + 0.01 * (2.0 * DRND(1) - 1.0);
			clamp_concentrations(c2h[i][j], c3h[i][j]);
		}
	}
}

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

void datsave(const char *filename)
{
	FILE *stream;
	int i, j;

	stream = fopen(filename, time1 == 0.0 ? "w" : "a");
	if (stream == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	fprintf(stream, "%f\n", time1);
	for (i = 0; i <= ndm; i++) {
		for (j = 0; j <= ndm; j++) {
			fprintf(stream, "%e  %e  ", c2h[i][j], c3h[i][j]);
		}
	}
	fprintf(stream, "\n");
	fclose(stream);
}

void save_bitmap_snapshot(int step)
{
	char filename[64];
	char path[512];

	graph_a();
	snprintf(filename, sizeof(filename), "%s%d.bmp", kBmpPrefix, step);
	snprintf(path, sizeof(path), "%s/%s", g_output_dir, filename);
	if (!save_screen(path)) {
		fprintf(stderr, "failed to save %s\n", path);
		exit(EXIT_FAILURE);
	}
}

}  // namespace
