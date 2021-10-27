#ifndef SUSI_DATA_TYPE_h
#define SUSI_DATA_TYPE_h

#include <stdint.h>

#include "SUSI_FN_BIT.h"
#include "SUSI_AUX_BIT.h"
#include "SUSI_AN_fn_BIT.h"


/* Identificativi della Direzione ricevuta dal Master */
#define	SUSI_DIRECTION		uint8_t
#define	SUSI_DIR_REV		0
#define	SUSI_DIR_FWD		1


/* Identificativi dei Gruppi Funzioni */
#define	SUSI_FN_GROUP		uint8_t
#define	SUSI_FN_0_4			1
#define	SUSI_FN_5_12		2
#define	SUSI_FN_13_20		3
#define	SUSI_FN_21_28		4
#define	SUSI_FN_29_36		5
#define	SUSI_FN_37_44		6
#define	SUSI_FN_45_52		7
#define	SUSI_FN_53_60		8
#define	SUSI_FN_61_68		9


/* Identificativi dei Gruppi AUXs */
#define	SUSI_AUX_GROUP		uint8_t
#define	SUSI_AUX_1_8		1
#define	SUSI_AUX_9_16		2
#define	SUSI_AUX_17_24		3
#define	SUSI_AUX_25_32		4


/* Identificativi dei Gruppi Funzioni Analogiche */
#define	SUSI_AN_GROUP		uint8_t
#define	SUSI_AN_FN_0_7		1
#define	SUSI_AN_FN_8_15		2
#define	SUSI_AN_FN_16_23	3
#define	SUSI_AN_FN_24_31	4
#define	SUSI_AN_FN_32_39	5
#define	SUSI_AN_FN_40_47	6
#define	SUSI_AN_FN_48_55	7
#define	SUSI_AN_FN_56_63	8


#endif
