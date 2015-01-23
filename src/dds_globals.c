#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "dds_globals.h"
/*
 * Returns true if the given
 * pointer is within the heap
 * (i.e. is malloc-ed)
 */
int in_heap(void *address){
	FILE *fp;
	char cmd[512];
	char result[24];
	result[23] = '\0';
	/* I strung together this abomination, which reads in the heap memory
	 * range from /proc/[PID]/maps and checks if the given address is inside of it.
	 */
	sprintf(cmd,"cat /proc/%d/maps |"
			" awk '{if($6==\"[heap]\") print $1}' |"
			" awk -F '[-, ]' '{if((0x%x > strtonum(\"0x\" $1)) && (0x%x < strtonum(\"0x\" $2))) print 1; else print 0}' |"
			" tr '\\n' ' ' | awk 'END{i=strtonum($1); for(f=0;f<=NF;f++){i = i || strtonum($f);} print i}'", getpid(), (int)address, (int)address);
	fp = popen(cmd, "r");
	if(!fp){
		printf("Failed to get heap address range. Exiting.\n");
		exit(1);
	}
	if(!(fgets(result,23,fp))){
		printf("Failed to copy heap range check into memory. Exiting.\n");
		exit(1);
	}
	pclose(fp);
	return result[0] == '1';
}

#ifdef ___TEST_SUITES___

void TestHeapCheck(CuTest *tc){
	int i = 7;
	int *j = &i;
	int *k = malloc(sizeof(i));
	*k = i;
	char s1[] = "Hello, World!";
	char *s2 = "Hello, World!";
	char *s3 = malloc(13 * sizeof(char));
	memcpy(s3,s1,13 * sizeof(char));
	CuAssertTrue(tc,!in_heap(j));
	CuAssertTrue(tc,in_heap(k));
	CuAssertTrue(tc,!in_heap(s1));
	CuAssertTrue(tc,!in_heap(s2));
	CuAssertTrue(tc,in_heap(s3));
	free(k);
	free(s3);
}

CuSuite *ParseHeapGetSuite(void){
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite,TestHeapCheck);
	return suite;
}

#endif
