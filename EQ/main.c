//////////////////////////////////////////////////////////////////////////////
// *
// * Predmetni projekat iz predmeta OAiS DSP 2
// * Godina: 2018
// *
// * Zadatak: Ekvalizacija audio signala
// * Autor: Marko Milosevic
// *                                                                          
// *                                                                          
/////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "ezdsp5535.h"
#include "ezdsp5535_i2c.h"
#include "aic3204.h"
#include "ezdsp5535_aic3204_dma.h"
#include "ezdsp5535_i2s.h"
#include "ezdsp5535_sar.h"
#include "print_number.h"
#include "math.h"

#include "iir.h"
#include "processing.h"

/* Frekvencija odabiranja */
#define SAMPLE_RATE 8000L

#define PI 3.14159265

#define K_025 8192
#define K_075 24576
#define DELTA_K 3277

#define ALPHA_LP 0.3   //ShelvingLP
#define ALPHA_HP -0.3  //ShelvingHP
#define ALPHA_P 0.7    //ShelvingPeek
#define BETA_P 0       //ShelvingPeek

#define ALPHA1_LP 0.888622125052771

#define BETA_P1 0.938191335922484
#define ALPHA1_P1 0.788336434585093

#define BETA_P2 0
#define ALPHA1_P2 0.414213562373095

#define ALPHA1_HP -0.854080685463467

/* Niz za smjestanje ulaznih i izlaznih odbiraka */
#pragma DATA_ALIGN(sampleBufferL,4)
Int16 sampleBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(sampleBufferR,4)
Int16 sampleBufferR[AUDIO_IO_SIZE];

Int16 output_LP[AUDIO_IO_SIZE];
Int16 output_HP[AUDIO_IO_SIZE];
Int16 output_P1[AUDIO_IO_SIZE];
Int16 output_P2[AUDIO_IO_SIZE];

Int16 output_LP_K025[AUDIO_IO_SIZE];
Int16 output_HP_K025[AUDIO_IO_SIZE];
Int16 output_LP_K075[AUDIO_IO_SIZE];
Int16 output_HP_K075[AUDIO_IO_SIZE];
Int16 output_P_K025[AUDIO_IO_SIZE];
Int16 output_P_K075[AUDIO_IO_SIZE];
Int16 peekCoeff[6];

Int16 shelvingCoeff_LP[4];
Int16 shelvingCoeff_HP[4];
Int16 peekCoeff1[6];
Int16 peekCoeff2[6];

Int16 x_history_L[2];
Int16 y_history_L[2];
Int16 x_history_H[2];
Int16 y_history_H[2];
Int16 x_history_P1[3];
Int16 y_history_P1[3];
Int16 x_history_P2[3];
Int16 y_history_P2[3];

Uint16 taster, prethodni_taster;

void printOnLcd(int opseg, Int16 pojacanje);

void main(void) {
	/* Inicijalizaija razvojne ploce */
	EZDSP5535_init();

	/* Inicijalizacija kontrolera za ocitavanje vrednosti pritisnutog dugmeta*/
	EZDSP5535_SAR_init();

	/* Inicijalizacija LCD kontrolera */
	initPrintNumber();

	printf("\n Ekvalizacija audio signala \n");

	/* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
	aic3204_hardware_init();

	/* Inicijalizacija AIC3204 kodeka */
	aic3204_init();

	aic3204_dma_init();

	/* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */
	set_sampling_frequency_and_gain(SAMPLE_RATE, 0);
	int i;
	for (i = 0; i < 2; i++) {
		x_history_L[i] = 0;
		y_history_L[i] = 0;
		x_history_H[i] = 0;
		y_history_H[i] = 0;
	}
	for (i = 0; i < 3; i++) {
		x_history_P1[i] = 0;
		y_history_P1[i] = 0;
		x_history_P2[i] = 0;
		y_history_P2[i] = 0;
	}

	/*Impulni odzivi za drugi, treci i cetvrti zadatak*/
	/*
	 calculateShelvingCoeff(ALPHA_LP, shelvingCoeff_LP);

	 for (i = 0; i < AUDIO_IO_SIZE; i++) {
	 output_LP_K025[i] = shelvingLP((i == 0) ? 16000 : 0, shelvingCoeff_LP,
	 x_history_L, y_history_L, K_025);
	 // output_LP_K075[i] = shelvingLP((i == 0) ? 16000 : 0, shelvingCoeff_LP,
	 //	 x_history_L, y_history_L, K_075);

	 }

	 calculateShelvingCoeff(ALPHA_HP, shelvingCoeff_HP);

	 for (i = 0; i < AUDIO_IO_SIZE; i++) {
	 output_HP_K025[i] = shelvingHP((i == 0) ? 16000 : 0, shelvingCoeff_HP,
	 x_history_H, y_history_H, K_025);
	 // output_HP_K075[i] = shelvingHP((i == 0) ? 16000 : 0, shelvingCoeff_HP,
	 //	 x_history_H, y_history_H, K_075);
	 }

	 calculatePeekCoeff(ALPHA_P, BETA_P, peekCoeff);
	 for (i = 0; i < 128; i++) {
	 output_P_K025[i] = shelvingPeek((i == 0) ? 16000 : 0, peekCoeff,
	 x_history_P1, y_history_P1, K_025);
	 //output_P_K075[i] = shelvingPeek((i == 0) ? 16000 : 0, peekCoeff, x_history_P,
	 //		y_history_P, K_075);
	 }
	 */
	taster = NoKey;
	prethodni_taster = NoKey;
	Int16 K[4] = { INT - 1, INT - 1, INT - 1, INT - 1 };

	int opseg = 1;

	calculateShelvingCoeff(ALPHA1_LP, shelvingCoeff_LP);
	calculatePeekCoeff(ALPHA1_P1, BETA_P1, peekCoeff1);
	calculatePeekCoeff(ALPHA1_P2, BETA_P2, peekCoeff2);
	calculateShelvingCoeff(ALPHA1_HP, shelvingCoeff_HP);
	printOnLcd(opseg, K[opseg - 1]);
	while (1) {
		aic3204_read_block(sampleBufferL, sampleBufferR);

		prethodni_taster = taster;
		taster = EZDSP5535_SAR_getKey();
		if (taster == SW1 && prethodni_taster != taster) {
			opseg += 1;
			if (opseg == 5) {
				opseg = 1;
			}
			printOnLcd(opseg, K[opseg - 1]);
		}
		if (taster == SW2 && prethodni_taster != taster) {
			if (opseg == 1) {

				K[opseg - 1] -= DELTA_K;
				if (K[opseg - 1] < 0) {
					K[opseg - 1] = INT - 1;
				}
				printOnLcd(opseg, K[opseg - 1]);
			}
			if (opseg == 2) {

				K[opseg - 1] -= DELTA_K;
				if (K[opseg - 1] < 0) {
					K[opseg - 1] = INT - 1;
				}
				printOnLcd(opseg, K[opseg - 1]);
			}
			if (opseg == 3) {

				K[opseg - 1] -= DELTA_K;
				if (K[opseg - 1] < 0) {
					K[opseg - 1] = INT - 1;
				}
				printOnLcd(opseg, K[opseg - 1]);

			}
			if (opseg == 4) {

				K[opseg - 1] -= DELTA_K;
				if (K[opseg - 1] < 0) {
					K[opseg - 1] = INT - 1;
				}
				printOnLcd(opseg, K[opseg - 1]);
			}

		}

		for (i = 0; i < AUDIO_IO_SIZE; i++) {
			output_LP[i] = shelvingLP(sampleBufferL[i], shelvingCoeff_LP,
					x_history_L, y_history_L, K[0]);
		}

		for (i = 0; i < AUDIO_IO_SIZE; i++) {
			output_P1[i] = shelvingPeek(output_LP[i], peekCoeff1, x_history_P1,
					y_history_P1, K[1]);
		}

		for (i = 0; i < AUDIO_IO_SIZE; i++) {
			output_P2[i] = shelvingPeek(output_P1[i], peekCoeff2, x_history_P2,
					y_history_P2, K[2]);
		}

		for (i = 0; i < AUDIO_IO_SIZE; i++) {
			output_HP[i] = shelvingHP(output_P2[i], shelvingCoeff_HP,
					x_history_H, y_history_H, K[3]);
		}

		aic3204_write_block(output_HP, sampleBufferR);
	}

	/* Prekid veze sa AIC3204 kodekom */
	aic3204_disable();

	printf("\n***Kraj programa***\n");
	SW_BREAKPOINT
	;
}

void printOnLcd(int opseg, Int16 pojacanje) {
	char opseg_c;
	char pojacanje_c;
	float pojacanje_cc = (float) pojacanje / INT * 10;
	pojacanje_c = ceil(pojacanje_cc);
	clearLCD();
	setWritePointerToFirstChar();

	opseg_c = opseg + 64;
	printChar(opseg_c);
	printChar(32);
	if (pojacanje_c == 10) {
		printChar('1');
		printChar('0');
	} else {
		printChar(pojacanje_c + 48);
	}

}

