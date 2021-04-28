#ifndef SUSI_DATA_TYPE_h
#define SUSI_DATA_TYPE_h

#include "SUSI_FN_BIT.h"
#include "SUSI_AUX_BIT.h"
#include "SUSI_AN_fn_BIT.h"

typedef enum {
	SUSI_DIR_REV = 0,						// direzione 'Reverse'
	SUSI_DIR_FWD = 1,						// 'ForWard'
} SUSI_DIRECTION;

typedef enum {
	SUSI_FN_0_4 = 1,
	SUSI_FN_5_12,
	SUSI_FN_13_20,
	SUSI_FN_21_28,
	SUSI_FN_29_36,
	SUSI_FN_37_44,
	SUSI_FN_45_52,
	SUSI_FN_53_60,
	SUSI_FN_61_68,
} SUSI_FN_GROUP;

typedef enum {
	SUSI_AUX_1_8 = 1,
	SUSI_AUX_9_16,
	SUSI_AUX_17_24,
	SUSI_AUX_25_32,
} SUSI_AUX_GROUP;

typedef enum {
	SUSI_AN_FN_0_7 = 1,
	SUSI_AN_FN_8_15,
	SUSI_AN_FN_16_23,
	SUSI_AN_FN_24_31,
	SUSI_AN_FN_32_39,
	SUSI_AN_FN_40_47,
	SUSI_AN_FN_48_55,
	SUSI_AN_FN_56_63,
} SUSI_AN_GROUP;

typedef struct {
	uint8_t Bytes[3];

	bool isCvManipulating;
	bool Complete;
} Rcn600_Message;

#endif