/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   testSi.h
 * Author: matheves
 *
 * Created on Nov 7, 2018, 9:31:03 AM
 */

#ifndef TESTSI_H
#define TESTSI_H

#include <cppunit/extensions/HelperMacros.h>

class TestSi : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(TestSi);

    CPPUNIT_TEST(testInterpreteur);
    CPPUNIT_TEST(testAnalyse);

    CPPUNIT_TEST_SUITE_END();

public:
    TestSi();
    virtual ~TestSi();
    void setUp();
    void tearDown();

private:
    void testInterpreteur();
    void testAnalyse();

};

#endif /* TESTSI_H */

