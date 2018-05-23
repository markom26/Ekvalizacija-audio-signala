#include "processing.h"
#include "iir.h"

void calculateShelvingCoeff(float c_alpha, Int16* output) {

	Int32 alpha = c_alpha * INT;

	if (alpha > 32767) {
		output[0] = 32767;
	} else {
		output[0] = (Int16) alpha;
	}

	if (-alpha > 32767) {
		output[3] = 32767;
	} else {
		output[3] = (Int16) -alpha;
	}

	output[1] = -INT;
	output[2] = INT - 1;

}

void calculatePeekCoeff(float c_alpha, float c_beta, Int16* output) {

	Int32 alpha = c_alpha * INT;
	Int32 beta = c_beta * (1 + c_alpha) * INT / 2;
	if (alpha > 32767) {
		output[0] = 32767;
		output[5] = 32767;
	} else {
		output[0] = (Int16) alpha;
		output[5] = (Int16) alpha;
	}

	if (-beta > 32767) {
		output[1] = 32767;
		output[4] = 32767;

	} else {
		output[1] = (Int16) -beta;
		output[4] = (Int16) -beta;
	}

	output[2] = INT - 1;
	output[3] = INT - 1;
}

Int16 shelvingHP(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history,
		Int16 k) {

	Int16 after_filtering;
	Int32 signal_H;
	Int32 signal_L;
	Int32 signal_H_k;
	Int32 result_32;
	Int16 result_16;

	after_filtering = first_order_IIR(input, coeff, x_history, y_history);

	signal_H = (after_filtering + (Int32) input) / 2;

	signal_L = ((Int32) input - after_filtering) / 2;

	signal_H_k = _smpy(signal_H, k);

	result_32 = signal_L + signal_H_k;

	if (result_32 > 32767) {
		result_16 = 32767;
	} else if (result_32 < -32768) {
		result_16 = -32768;
	} else {
		result_16 = (Int16) result_32;
	}

	return result_16;

}

Int16 shelvingLP(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history,
		Int16 k) {

	Int16 after_filtering;
	Int32 signal_H;
	Int32 signal_L;
	Int32 signal_L_k;
	Int32 result_32;
	Int16 result_16;

	after_filtering = first_order_IIR(input, coeff, x_history, y_history);

	signal_H = (after_filtering + (Int32) input) >> 1;

	signal_L = ((Int32) input - after_filtering) >> 1;

	signal_L_k = _smpy(signal_L, k);

	result_32 = signal_H + signal_L_k;

	if (result_32 > 32767) {
		result_16 = 32767;
	} else if (result_32 < -32768) {
		result_16 = -32768;
	} else {
		result_16 = (Int16) result_32;
	}

	return result_16;
}

Int16 shelvingPeek(Int16 input, Int16* coeff, Int16* x_history,
		Int16* y_history, Int16 k) {

	Int16 after_filtering;
	Int32 signal_H;
	Int32 signal_L;
	Int32 signal_L_k;
	Int32 result_32;
	Int16 result_16;

	after_filtering = second_order_IIR(input, coeff, x_history, y_history);

	signal_H = (after_filtering + (Int32) input) / 2;

	signal_L = ((Int32) input - after_filtering) / 2;

	signal_L_k = _smpy(signal_L, k);

	result_32 = signal_H + signal_L_k;

	if (result_32 > 32767) {
		result_16 = 32767;
	} else if (result_32 < -32768) {
		result_16 = -32768;
	} else {
		result_16 = (Int16) result_32;
	}

	return result_16;
}

