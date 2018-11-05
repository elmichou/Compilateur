#ifndef ARBREABSTRAIT_H
#define ARBREABSTRAIT_H

// Contient toutes les déclarations de classes nécessaires
//  pour représenter l'arbre abstrait

#include <vector>
#include <iostream>
#include <iomanip>
using namespace std;

#include "Symbole.h"
#include "Exceptions.h"

////////////////////////////////////////////////////////////////////////////////
class Noeud {
// Classe abstraite dont dériveront toutes les classes servant à représenter l'arbre abstrait
// Remarque : la classe ne contient aucun constructeur
  public:
    virtual int  executer() =0 ; // Méthode pure (non implémentée) qui rend la classe abstraite
    virtual void ajoute(Noeud* instruction) { throw OperationInterditeException(); }
    virtual ~Noeud() {} // Présence d'un destructeur virtuel conseillée dans les classes abstraites
    virtual void traduitEnCPP(ostream & cout,unsigned int indentation)const=0;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudSeqInst : public Noeud {
// Classe pour représenter un noeud "sequence d'instruction"
//  qui a autant de fils que d'instructions dans la séquence
  public:
     NoeudSeqInst();   // Construit une séquence d'instruction vide
    ~NoeudSeqInst() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();    // Exécute chaque instruction de la séquence
    void ajoute(Noeud* instruction);  // Ajoute une instruction à la séquence
    void traduitEnCPP(ostream & cout,unsigned int indentation) const;

  private:
    vector<Noeud *> m_instructions; // pour stocker les instructions de la séquence
};

////////////////////////////////////////////////////////////////////////////////
class NoeudAffectation : public Noeud {
// Classe pour représenter un noeud "affectation"
//  composé de 2 fils : la variable et l'expression qu'on lui affecte
  public:
     NoeudAffectation(Noeud* variable, Noeud* expression); // construit une affectation
    ~NoeudAffectation() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();        // Exécute (évalue) l'expression et affecte sa valeur à la variable
    void traduitEnCPP(ostream & cout,unsigned int indentation) const;

  private:
    Noeud* m_variable;
    Noeud* m_expression;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudOperateurBinaire : public Noeud {
// Classe pour représenter un noeud "opération binaire" composé d'un opérateur
//  et de 2 fils : l'opérande gauche et l'opérande droit
  public:
    NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit);
    // Construit une opération binaire : operandeGauche operateur OperandeDroit
   ~NoeudOperateurBinaire() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();            // Exécute (évalue) l'opération binaire)
    void traduitEnCPP(ostream & cout,unsigned int indentation) const;

  private:
    Symbole m_operateur;
    Noeud*  m_operandeGauche;
    Noeud*  m_operandeDroit;
};

////////////////////////////////////////////////////////////////////////////////
class NoeudInstSi : public Noeud {
// Classe pour représenter un noeud "instruction si"
//  et ses 2 fils : la condition du si et la séquence d'instruction associée
  public:
    NoeudInstSi(Noeud* condition, Noeud* sequence);
     // Construit une "instruction si" avec sa condition et sa séquence d'instruction
   ~NoeudInstSi() {} // A cause du destructeur virtuel de la classe Noeud
    int executer();  // Exécute l'instruction si : si condition vraie on exécute la séquence
    void traduitEnCPP(ostream & cout,unsigned int indentation) const;

  private:
    Noeud*  m_condition;
    Noeud*  m_sequence;
};

class NoeudInstSiRiche : public Noeud {
// Classe pour représenter un noeud "instruction siRiche"
//  et ses multiples fils : la condition du si et la séquence d'instruction associée
public:
    NoeudInstSiRiche(vector<Noeud*> conditions, vector<Noeud*> sequences); //construit un si riche avec ses conditions et ses séquences d'instructions
    ~NoeudInstSiRiche() {}; // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute l'instruction si : si condition vraie on exécute la séquence ou executer sinon
    void traduitEnCPP(ostream & cout,unsigned int indentation) const;
    
private:
    vector<Noeud*> m_conditions;
    vector<Noeud*> m_sequences;
    
};

class NoeudInstTantQue : public Noeud{
// Classe pour représenter un noeud "instruction tantque"
//  et ses multiples fils : la condition du tantque et la séquence d'instruction associée
public:
    NoeudInstTantQue(Noeud* condition, Noeud* sequence); //construit un tant que avec sa condition et sa séquence d'instructions
    ~NoeudInstTantQue() {}; // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute l'instruction si : si condition vraie on exécute la séquence tant qu'elle n'est pas fausse
    void traduitEnCPP(ostream & cout,unsigned int indentation) const;
    
private:
    Noeud* m_condition;
    Noeud* m_sequence;
};

class NoeudInstRepeter : public Noeud{
// Classe pour représenter un noeud "instruction repeter"
//  et ses multiples fils : la condition du jusqua et la séquence d'instruction associée
public:
    NoeudInstRepeter(Noeud* condition, Noeud* sequence); //construit un repeter avec sa condition et sa séquence d'instructions
    ~NoeudInstRepeter() {}; // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute l'instruction si : si condition vraie on exécute la séquence tant qu'elle n'est pas fausse
    void traduitEnCPP(ostream & cout,unsigned int indentation) const;
    
private:
    Noeud* m_condition;
    Noeud* m_sequence;
};

class NoeudInstPour : public Noeud{
// Classe pour représenter un noeud "instruction pour"
// et ses multiples fils :l'affectation, la condition du pour, la 2e affectation et la séquence d'instruction associée
public:
    NoeudInstPour(vector<Noeud*> affectations, Noeud* condition, Noeud* sequence); //on construit un pour avec ses affactation, sa condition et sa séquence d'instruction associée
    ~NoeudInstPour() {}; // A cause du destructeur virtuel de la classe Noeud
    int executer(); // Exécute l'instruction
    void traduitEnCPP(ostream & cout,unsigned int indentation) const;
    
private:
    vector<Noeud*> m_affectations;
    Noeud* m_condition;
    Noeud* m_sequence;    
};

class NoeudChaine : public Noeud {
public:
    NoeudChaine(string chaine);
 
    ~NoeudChaine() {
    }
 
    const string getChaine() const;
 
    int executer();
 
private:
    string m_chaine;
};

class NoeudInstEcrire : public Noeud {
public:
    NoeudInstEcrire(vector<Noeud*> container);
 
    ~NoeudInstEcrire() {
    }
    int executer();
    
    void traduitEnCPP(ostream & cout,unsigned int indentation) const;
 
private:
    vector<Noeud*> m_container;
};

class NoeudValeur : public Noeud{
public:
    NoeudValeur(int val);
    ~NoeudValeur(){}
    int executer();
private:
    int m_val;
};

class NoeudInstLire : public Noeud {
// Classe pour représenter un noeud "instruction lire"
//  et son fils : noeud *
  public:
    NoeudInstLire(Noeud* noeud, std::vector<Noeud*> noeuds);
     // Construit une instruction lire avec le noeud mis en paramètre.
   ~NoeudInstLire() {}; // A cause du destructeur virtuel de la classe Noeud
    int executer();  // Exécute l'instruction lire : lis les variables inscrits en paramètres
    void traduitEnCPP(ostream & cout,unsigned int indentation) const;    

  private:
      Noeud* m_noeud;
      std::vector<Noeud*> m_noeudsSupp;
};


#endif /* ARBREABSTRAIT_H */


