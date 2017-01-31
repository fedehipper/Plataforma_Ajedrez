#include <CUnit/Basic.h>
#include "../src/movimientos_torre.h"

void test_es_amigo_de_torre(void) {
	CU_ASSERT(es_amigo_de_torre('t', 'c'));
	CU_ASSERT(es_amigo_de_torre('t', 'a'));
	CU_ASSERT(es_amigo_de_torre('t', 't'));
	CU_ASSERT(es_amigo_de_torre('t', 'w'));
	CU_ASSERT(es_amigo_de_torre('t', 'r'));
	CU_ASSERT(es_amigo_de_torre('t', 'p'));
	CU_ASSERT(es_amigo_de_torre('T', 'C'));
	CU_ASSERT(es_amigo_de_torre('T', 'A'));
	CU_ASSERT(es_amigo_de_torre('T', 'T'));
	CU_ASSERT(es_amigo_de_torre('T', 'W'));
	CU_ASSERT(es_amigo_de_torre('T', 'R'));
	CU_ASSERT(es_amigo_de_torre('T', 'P'));
	CU_ASSERT_FALSE(es_amigo_de_torre('T', 'c'));
	CU_ASSERT_FALSE(es_amigo_de_torre('T', 'a'));
	CU_ASSERT_FALSE(es_amigo_de_torre('T', 't'));
	CU_ASSERT_FALSE(es_amigo_de_torre('T', 'w'));
	CU_ASSERT_FALSE(es_amigo_de_torre('T', 'r'));
	CU_ASSERT_FALSE(es_amigo_de_torre('T', 'p'));
	CU_ASSERT_FALSE(es_amigo_de_torre('t', 'C'));
	CU_ASSERT_FALSE(es_amigo_de_torre('t', 'A'));
	CU_ASSERT_FALSE(es_amigo_de_torre('t', 'T'));
	CU_ASSERT_FALSE(es_amigo_de_torre('t', 'W'));
	CU_ASSERT_FALSE(es_amigo_de_torre('t', 'R'));
	CU_ASSERT_FALSE(es_amigo_de_torre('t', 'P'));
}