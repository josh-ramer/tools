#ifndef TEST_H
#define TEST_H

#include <setjmp.h>
#include <stdarg.h>

#define TEST_VERSION  "Test 1.5b"

/* String */

char* StrAlloc(int size);
char* StrCopy(const char* old);

#define ALLOC(TYPE)		((TYPE*) malloc(sizeof(TYPE)))

#define HUGE_STRING_LEN	8192
#define STRING_MAX		256
#define STRING_INC		256

typedef struct
{
	int length;
	int size;
	char* buffer;
} String;

void StringInit(String* str);
String* StringNew(void);
void StringRead(String* str, const char* path);
void StringAppend(String* str, const char* text);
void StringAppendChar(String* str, char ch);
void StringAppendFormat(String* str, const char* format, ...);
void StringInsert(String* str, const char* text, int pos);
void StringResize(String* str, int newSize);
void StringDelete(String* str);

/* Test */

typedef struct Test Test;

typedef void (*TestFunction)(Test *);

struct Test
{
	char* name;
	TestFunction function;
	int failed;
	int ran;
	String *message;
	jmp_buf *jumpBuf;
};

void TestInit(Test* t, const char* name, TestFunction function);
Test* TestNew(const char* name, TestFunction function);
void TestRun(Test* tc);
void TestDelete(Test *t);

/* Internal versions of assert functions -- use the public versions */
void Fail_Line(Test* tc, const char* file, int line, const char* message2, const char* message);
void Assert_Line(Test* tc, const char* file, int line, const char* message, int condition);
void AssertStrEquals_LineMsg(Test* tc, 
	const char* file, int line, const char* message, 
	const char* expected, const char* actual);
void AssertIntEquals_LineMsg(Test* tc, 
	const char* file, int line, const char* message, 
	int expected, int actual);
void AssertIntArrayEquals_LineMsg(Test* tc, 
	const char* file, int line, const char* message, 
	const int expected[], const int actual[], int length);
void AssertDblEquals_LineMsg(Test* tc, 
	const char* file, int line, const char* message, 
	double expected, double actual, double delta);
void AssertPtrEquals_LineMsg(Test* tc, 
	const char* file, int line, const char* message, 
	void* expected, void* actual);

/* public assert functions */

#define Fail(tc, ms)                        Fail_Line(  (tc), __FILE__, __LINE__, NULL, (ms))
#define Assert(tc, ms, cond)                Assert_Line((tc), __FILE__, __LINE__, (ms), (cond))
#define AssertTrue(tc, cond)                Assert_Line((tc), __FILE__, __LINE__, "assert failed", (cond))

#define AssertStrEquals(tc,ex,ac)           AssertStrEquals_LineMsg((tc),__FILE__,__LINE__,NULL,(ex),(ac))
#define AssertStrEquals_Msg(tc,ms,ex,ac)    AssertStrEquals_LineMsg((tc),__FILE__,__LINE__,(ms),(ex),(ac))
#define AssertIntEquals(tc,ex,ac)           AssertIntEquals_LineMsg((tc),__FILE__,__LINE__,NULL,(ex),(ac))
#define AssertIntArrayEquals(tc,ex,ac,ca)           AssertIntArrayEquals_LineMsg((tc),__FILE__,__LINE__,NULL,(ex),(ac),(ca))
#define AssertIntEquals_Msg(tc,ms,ex,ac)    AssertIntEquals_LineMsg((tc),__FILE__,__LINE__,(ms),(ex),(ac))
#define AssertDblEquals(tc,ex,ac,dl)        AssertDblEquals_LineMsg((tc),__FILE__,__LINE__,NULL,(ex),(ac),(dl))
#define AssertDblEquals_Msg(tc,ms,ex,ac,dl) AssertDblEquals_LineMsg((tc),__FILE__,__LINE__,(ms),(ex),(ac),(dl))
#define AssertPtrEquals(tc,ex,ac)           AssertPtrEquals_LineMsg((tc),__FILE__,__LINE__,NULL,(ex),(ac))
#define AssertPtrEquals_Msg(tc,ms,ex,ac)    AssertPtrEquals_LineMsg((tc),__FILE__,__LINE__,(ms),(ex),(ac))

#define AssertPtrNotNull(tc,p)        Assert_Line((tc),__FILE__,__LINE__,"null pointer unexpected",((p) != NULL))
#define AssertPtrIsNull(tc,p)        Assert_Line((tc),__FILE__,__LINE__,"null pointer unexpected",((p) == NULL))
#define AssertPtrNotNullMsg(tc,msg,p) Assert_Line((tc),__FILE__,__LINE__,(msg),((p) != NULL))

/* Suite */

#define MAX_TEST_CASES	1024

#define SUITE_ADD_TEST(SUITE,TEST)	TestSuiteAdd(SUITE, TestNew(#TEST, TEST))

typedef struct
{
	int count;
	Test* list[MAX_TEST_CASES];
	int failCount;

} TestSuite;


void TestSuiteInit(TestSuite* testSuite);
TestSuite* TestSuiteNew(void);
void TestSuiteDelete(TestSuite* testSuite);
void TestSuiteAdd(TestSuite* testSuite, Test *testCase);
void TestSuiteAddSuite(TestSuite* testSuite, TestSuite* testSuite2);
void TestSuiteRun(TestSuite* testSuite);
void TestSuiteSummary(TestSuite* testSuite, String* summary);
void TestSuiteDetails(TestSuite* testSuite, String* details);

#endif /* TEST_H */