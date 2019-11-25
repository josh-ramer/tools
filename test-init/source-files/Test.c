#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Test.h"

/*-------------------------------------------------------------------------*
*********PLACE THIS IN TESTING FILE IOT RUN TESTS AND RETRIEVE SUMMARY***************
	TestSuite *tests = TestSuiteNew();
	String *output = StringNew();

	TestSuiteRun(tests);
	TestSuiteSummary(tests, output);
	TestSuiteDetails(tests, output);
	printf("%s\n", output->buffer);
	printf("Fail Count: %d\n", tests->failCount);
	TestSuiteDelete(tests);
 *-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*
 * Str
 *-------------------------------------------------------------------------*/

char* StrAlloc(int size)
{
	char* newStr = (char*) malloc( sizeof(char) * (size) );
	return newStr;
}

char* StrCopy(const char* old)
{
	int len = strlen(old);
	char* newStr = StrAlloc(len + 1);
	strcpy(newStr, old);
	return newStr;
}

/*-------------------------------------------------------------------------*
 * String
 *-------------------------------------------------------------------------*/

void StringInit(String* str)
{
	str->length = 0;
	str->size = STRING_MAX;
	str->buffer = (char*) malloc(sizeof(char) * str->size);
	str->buffer[0] = '\0';
}

String* StringNew(void)
{
	String* str = (String*) malloc(sizeof(String));
	str->length = 0;
	str->size = STRING_MAX;
	str->buffer = (char*) malloc(sizeof(char) * str->size);
	str->buffer[0] = '\0';
	return str;
}

void StringDelete(String *str)
{
        if (!str) return;
        free(str->buffer);
        free(str);
}

void StringResize(String* str, int newSize)
{
	str->buffer = (char*) realloc(str->buffer, sizeof(char) * newSize);
	str->size = newSize;
}

void StringAppend(String* str, const char* text)
{
	int length;

	if (text == NULL) {
		text = "NULL";
	}

	length = strlen(text);
	if (str->length + length + 1 >= str->size)
		StringResize(str, str->length + length + 1 + STRING_INC);
	str->length += length;
	strcat(str->buffer, text);
}

void StringAppendChar(String* str, char ch)
{
	char text[2];
	text[0] = ch;
	text[1] = '\0';
	StringAppend(str, text);
}

void StringAppendFormat(String* str, const char* format, ...)
{
	va_list argp;
	char buf[HUGE_STRING_LEN];
	va_start(argp, format);
	vsprintf(buf, format, argp);
	va_end(argp);
	StringAppend(str, buf);
}

void StringInsert(String* str, const char* text, int pos)
{
	int length = strlen(text);
	if (pos > str->length)
		pos = str->length;
	if (str->length + length + 1 >= str->size)
		StringResize(str, str->length + length + 1 + STRING_INC);
	memmove(str->buffer + pos + length, str->buffer + pos, (str->length - pos) + 1);
	str->length += length;
	memcpy(str->buffer + pos, text, length);
}

/*-------------------------------------------------------------------------*
 * Test
 *-------------------------------------------------------------------------*/

void TestInit(Test* t, const char* name, TestFunction function)
{
	t->name = StrCopy(name);
	t->failed = 0;
	t->ran = 0;
        t->message = NULL;
	t->function = function;
	t->jumpBuf = NULL;
}

Test* TestNew(const char* name, TestFunction function)
{
	Test* tc = ALLOC(Test);
	TestInit(tc, name, function);
	return tc;
}

void TestDelete(Test *t)
{
        if (!t) return;
        StringDelete(t->message);
        free(t->name);
        free(t);
}

void TestRun(Test* tc)
{
	jmp_buf buf;
	tc->jumpBuf = &buf;
	if (setjmp(buf) == 0)
	{
		tc->ran = 1;
		(tc->function)(tc);
	}
	tc->jumpBuf = 0;
}

static void FailInternal(Test* tc, const char* file, int line, String* string)
{
	char buf[HUGE_STRING_LEN];

	sprintf(buf, "%s:%d: ", file, line);
	StringInsert(string, buf, 0);

	tc->failed = 1;
        free(tc->message);
        tc->message = StringNew();
        StringAppend(tc->message, string->buffer);
	if (tc->jumpBuf != 0) longjmp(*(tc->jumpBuf), 0);
}

void Fail_Line(Test* tc, const char* file, int line, const char* message2, const char* message)
{
	String string;

	StringInit(&string);
	if (message2 != NULL) 
	{
		StringAppend(&string, message2);
		StringAppend(&string, ": ");
	}
	StringAppend(&string, message);
	FailInternal(tc, file, line, &string);
}

void Assert_Line(Test* tc, const char* file, int line, const char* message, int condition)
{
	if (condition) return;
	Fail_Line(tc, file, line, NULL, message);
}

void Assert_True(Test* tc, const char* file, int line, const char* message, int condition)
{
	if (condition) return;
	Fail_Line(tc, file, line, NULL, message);
}

void Assert_False(Test* tc, const char* file, int line, const char* message, int condition)
{
	if (!condition) return;
	Fail_Line(tc, file, line, NULL, message);
}

void AssertStrEquals_LineMsg(Test* tc, const char* file, int line, const char* message, 
	const char* expected, const char* actual)
{
	String string;
	if ((expected == NULL && actual == NULL) ||
	    (expected != NULL && actual != NULL &&
	     strcmp(expected, actual) == 0))
	{
		return;
	}

	StringInit(&string);
	if (message != NULL) 
	{
		StringAppend(&string, message);
		StringAppend(&string, ": ");
	}
	StringAppend(&string, "expected <");
	StringAppend(&string, expected);
	StringAppend(&string, "> but was <");
	StringAppend(&string, actual);
	StringAppend(&string, ">");
	FailInternal(tc, file, line, &string);
}

void AssertIntEquals_LineMsg(Test* tc, const char* file, int line, const char* message, 
	int expected, int actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected <%d> but was <%d>", expected, actual);
	Fail_Line(tc, file, line, message, buf);
}

void AssertIntArrayEquals_LineMsg(Test* tc, const char* file, int line, const char* message, 
	const int expected[], const int actual[], int length)
{
	char buf[STRING_MAX];
	int pass=1;
	for(int i=0; i<length/sizeof(int); i++) {
		if(expected[i] != actual[i]) {
			sprintf(buf, "expected array first differed from actual array on term %d", i);
			pass=0;
			break;
		}
	}
	if (pass) return;
	Fail_Line(tc, file, line, message, buf);
}

void AssertDblEquals_LineMsg(Test* tc, const char* file, int line, const char* message, 
	double expected, double actual, double delta)
{
	char buf[STRING_MAX];
	if (fabs(expected - actual) <= delta) return;
	sprintf(buf, "expected <%f> but was <%f>", expected, actual); 

	Fail_Line(tc, file, line, message, buf);
}

void AssertPtrEquals_LineMsg(Test* tc, const char* file, int line, const char* message, 
	void* expected, void* actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected pointer <0x%p> but was <0x%p>", expected, actual);
	Fail_Line(tc, file, line, message, buf);
}

/*-------------------------------------------------------------------------*
 * TestSuite
 *-------------------------------------------------------------------------*/

void TestSuiteInit(TestSuite* testSuite)
{
	testSuite->count = 0;
	testSuite->failCount = 0;
        memset(testSuite->list, 0, sizeof(testSuite->list));
}

TestSuite* TestSuiteNew(void)
{
	TestSuite* testSuite = ALLOC(TestSuite);
	TestSuiteInit(testSuite);
	return testSuite;
}

void TestSuiteDelete(TestSuite *testSuite)
{
        unsigned int n;
        for (n=0; n < MAX_TEST_CASES; n++)
        {
                if (testSuite->list[n])
                {
                        TestDelete(testSuite->list[n]);
                }
        }
        free(testSuite);

}

void TestSuiteAdd(TestSuite* testSuite, Test *testCase)
{
	assert(testSuite->count < MAX_TEST_CASES);
	testSuite->list[testSuite->count] = testCase;
	testSuite->count++;
}

void SuiteAddSuite(TestSuite* testSuite, TestSuite* testSuite2)
{
	int i;
	for (i = 0 ; i < testSuite2->count ; ++i)
	{
		Test* testCase = testSuite2->list[i];
		TestSuiteAdd(testSuite, testCase);
	}
}

void TestSuiteRun(TestSuite* testSuite)
{
	int i;
	for (i = 0 ; i < testSuite->count ; ++i)
	{
		Test* testCase = testSuite->list[i];
		TestRun(testCase);
		if (testCase->failed) { testSuite->failCount += 1; }
	}
}

void TestSuiteSummary(TestSuite* testSuite, String* summary)
{
	int i;
	for (i = 0 ; i < testSuite->count ; ++i)
	{
		Test* testCase = testSuite->list[i];
		StringAppend(summary, testCase->failed ? "F" : ".");
	}
	StringAppend(summary, "\n\n");
}

void TestSuiteDetails(TestSuite* testSuite, String* details)
{
	int i;
	int failCount = 0;

	if (testSuite->failCount == 0)
	{
		int passCount = testSuite->count - testSuite->failCount;
		const char* testWord = passCount == 1 ? "test" : "tests";
		StringAppendFormat(details, "OK (%d %s)\n", passCount, testWord);
	}
	else
	{
		if (testSuite->failCount == 1)
			StringAppend(details, "There was 1 failure:\n");
		else
			StringAppendFormat(details, "There were %d failures:\n", testSuite->failCount);

		for (i = 0 ; i < testSuite->count ; ++i)
		{
			Test* testCase = testSuite->list[i];
			if (testCase->failed)
			{
				failCount++;
				StringAppendFormat(details, "%d) %s: %s\n",
					failCount, testCase->name, testCase->message->buffer);
			}
		}
		StringAppend(details, "\n!!!FAILURES!!!\n");

		StringAppendFormat(details, "Runs: %d ",   testSuite->count);
		StringAppendFormat(details, "Passes: %d ", testSuite->count - testSuite->failCount);
		StringAppendFormat(details, "Fails: %d\n",  testSuite->failCount);
	}
}