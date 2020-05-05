/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   testSi.cpp
 * Author: matheves
 *
 * Created on Nov 7, 2018, 9:31:03 AM
 */

#include "TestSi.h"
#include "../Interpreteur.h"


CPPUNIT_TEST_SUITE_REGISTRATION(TestSi);

TestSi::TestSi() {
}

TestSi::~TestSi() {
}

void TestSi::setUp() {
}

void TestSi::tearDown() {
}

void TestSi::testInterpreteur() {
    string nomFich="testSi.txt";
    //test langage
    ifstream fichier(nomFich.c_str());
    Interpreteur interpreteur(fichier);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Aucune erreur d'interpretation : ",interpreteur.analyse());
    
    string nomFich2="testSiErreur.txt";
    ifstream fichier2(nomFich2.c_str());
    Interpreteur interpreteur2(fichier2);
    CPPUNIT_ASSERT_THROW_MESSAGE("Erreur d'interpretation : ",interpreteur2.analyse(),SyntaxeException);
}

void TestSi::testAnalyse() {
    string nomFich="testSi.txt";
    
    ifstream fichier(nomFich.c_str());
    Interpreteur interpreteur(fichier);
    interpreteur.analyse();
    
    if(interpreteur.getArbre()!=nullptr){
        interpreteur.getArbre()->executer();
        TableSymboles laTable=interpreteur.getTable();
        //Test sur l'execution des noeuds
        const Symbole test1("test1");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verification test1==2",2,laTable.chercheAjoute(Symbole("test1"))->executer());
        
        const Symbole test2("test2");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Verification test2==4",4,laTable.chercheAjoute(Symbole("test2"))->executer());
    }
}

