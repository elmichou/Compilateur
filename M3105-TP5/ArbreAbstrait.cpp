#include <stdlib.h>
#include <typeinfo>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"

////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////
NoeudSeqInst::NoeudSeqInst() : m_instructions() {
 }
 
 int NoeudSeqInst::executer() {
   for (unsigned int i = 0; i < m_instructions.size(); i++)
     m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
   return 0; // La valeur renvoyée ne représente rien !
 }
 
 void NoeudSeqInst::ajoute(Noeud* instruction) {
   if (instruction!=nullptr) m_instructions.push_back(instruction);
 }
 
 void NoeudSeqInst::traduitEnCPP(ostream & cout,unsigned int indentation) const {
     for (unsigned int i = 0; i < m_instructions.size(); i++) {
         m_instructions[i]->traduitEnCPP(cout, indentation); // on exécute chaque instruction de la séquence
         if((typeid(*m_instructions[i]))==(typeid(NoeudAffectation))){
             cout << ";" << endl;
         }else {
             cout << endl; // La valeur renvoyée ne représente rien !
         }
     }
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
  int valeur = m_expression->executer(); // On exécute (évalue) l'expression
  ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudAffectation::traduitEnCPP(ostream & cout,unsigned int indentation) const {
    m_variable->traduitEnCPP(cout,indentation);
    cout << " = ";
    m_expression->traduitEnCPP(cout, 0);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
  int og, od, valeur;
  if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
  if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
  // Et on combine les deux opérandes en fonctions de l'opérateur
  if (this->m_operateur == "+") valeur = (og + od);
  else if (this->m_operateur == "-") valeur = (og - od);
  else if (this->m_operateur == "*") valeur = (og * od);
  else if (this->m_operateur == "==") valeur = (og == od);
  else if (this->m_operateur == "!=") valeur = (og != od);
  else if (this->m_operateur == "<") valeur = (og < od);
  else if (this->m_operateur == ">") valeur = (og > od);
  else if (this->m_operateur == "<=") valeur = (og <= od);
  else if (this->m_operateur == ">=") valeur = (og >= od);
  else if (this->m_operateur == "et") valeur = (og && od);
  else if (this->m_operateur == "ou") valeur = (og || od);
  else if (this->m_operateur == "non") valeur = (!og);
  else if (this->m_operateur == "/") {
    if (od == 0) throw DivParZeroException();
    valeur = og / od;
  }
  return valeur; // On retourne la valeur calculée
}

void NoeudOperateurBinaire::traduitEnCPP(ostream & cout,unsigned int indentation) const {
    m_operandeGauche->traduitEnCPP(cout,0);
    if (m_operateur.getChaine() == "et"){
        cout << " && ";
    } else if (m_operateur.getChaine() == "ou"){
        cout << " || ";
    } else {
        cout << " " << m_operateur.getChaine() << " ";
    }
    
    m_operandeDroit->traduitEnCPP(cout, 0);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence) {
}

int NoeudInstSi::executer() {
  if (m_condition->executer()) m_sequence->executer();
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudInstSi::traduitEnCPP(ostream & cout,unsigned int indentation)const{
  cout << setw(4*indentation)<<""<<"if (";// Ecrit "if (" avec un décalage de 4*indentation espaces 
  m_condition->traduitEnCPP(cout,0);// Traduit la condition en C++ sans décalage 
  cout <<") {"<< endl;// Ecrit ") {" et passe à la ligne 
  m_sequence->traduitEnCPP(cout, indentation+1);// Traduit en C++ la séquence avec indentation augmentée 
  cout << setw(4*indentation)<<""<<"}"<< endl;// Ecrit "}" avec l'indentation initiale et passe à la ligne 
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSiRiche
////////////////////////////////////////////////////////////////////////////////

NoeudInstSiRiche::NoeudInstSiRiche(vector<Noeud*> conditions, vector<Noeud*> sequences)
: m_conditions(conditions), m_sequences(sequences){
}

int NoeudInstSiRiche::executer() {
    int i = 0;
    while(i < m_conditions.size() && !m_conditions[i]->executer()){
        i++;
    }
    if(m_conditions.size() == m_sequences.size() && i == m_conditions.size()){
        return 0;
    }
    m_sequences[i]->executer();
    return 0;
}

void NoeudInstSiRiche::traduitEnCPP(ostream & cout,unsigned int indentation) const {
    unsigned int i = 1;
    cout << setw(4*indentation)<<""<<"if (";// Ecrit "if (" avec un décalage de 4*indentation d'espaces 
    m_conditions.at(0)->traduitEnCPP(cout,0);// Traduit la condition en C++ sans décalage 
    cout << ") {"<< endl;// Ecrit ") {" et passe à la ligne  
    m_sequences.at(0)->traduitEnCPP(cout, indentation+1);// Traduit en C++ la séquence avec indentation augmentée 
    cout << setw(4*indentation)<< "" << "}";// Ecrit "}" avec l'indentation initiale et passe à la ligne 
    
    while(i < m_conditions.size()){ // on parcours le vecteur
         // si on n'est pas sur le dernier élément
        cout <<"else if ("; // il écrite à la suite de la précédente accolade
        m_conditions.at(i)->traduitEnCPP(cout,0);
        cout << ") {" << endl;
        m_sequences.at(i)->traduitEnCPP(cout,indentation+1);
        cout << setw(4*indentation)<<""<<"}";
        i++;
        
    }
    
    if(m_sequences.size() != m_conditions.size()){ // si il y a un sinon
        cout<<"else {"<<endl;
        m_sequences.at(m_sequences.size()-1)->traduitEnCPP(cout,indentation+1);
        cout <<setw(4*indentation) <<""<< "}" ;
    }
    
}

////////////////////////////////////////////////////////////////////////////////
// NoeudTantQue
////////////////////////////////////////////////////////////////////////////////

NoeudInstTantQue::NoeudInstTantQue(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence){
}

int NoeudInstTantQue::executer() {
    while(m_condition->executer()){
        m_sequence->executer();
    }
    return 0;
}

void NoeudInstTantQue::traduitEnCPP(ostream & cout,unsigned int indentation) const {
    cout << setw(4*indentation)<<""<<"while ("; 
    m_condition->traduitEnCPP(cout, 0);// on met la condition d'arrêt dans les paramètres du while
    cout <<") {" <<endl;
    m_sequence->traduitEnCPP(cout, indentation+1); // on met la séquence on augmentant l'indentation
    cout << setw(4*indentation)<<""<<"}";
}

////////////////////////////////////////////////////////////////////////////////
// NoeudRepeter
////////////////////////////////////////////////////////////////////////////////

NoeudInstRepeter::NoeudInstRepeter(Noeud* condition, Noeud* sequence)
: m_condition(condition), m_sequence(sequence){
}

int NoeudInstRepeter::executer() {
    do{
      m_sequence->executer();
    }while(!m_condition->executer());
    return 0;
}

void NoeudInstRepeter::traduitEnCPP(ostream & cout,unsigned int indentation) const {
    cout << setw(4*indentation)<<""<< "do {" << endl; // écris do avec un espace de 4* indentation
    m_sequence->traduitEnCPP(cout, indentation+1);
    cout << setw(4*indentation)<<""<< "} while (";
    m_condition->traduitEnCPP(cout, 0);
    cout << ");";
}

////////////////////////////////////////////////////////////////////////////////
// NoeudPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(vector<Noeud*> affectations, Noeud* condition, Noeud* sequence)
: m_affectations(affectations), m_condition(condition), m_sequence(sequence){
}

int NoeudInstPour::executer(){
    if(m_affectations[0] == nullptr && m_affectations[1] == nullptr){
        for(; m_condition->executer();){
            m_sequence->executer();
        }
    } else if(m_affectations[0] == nullptr){
        for(; m_condition->executer(); m_affectations[1]->executer()){
            m_sequence->executer();
        }
    } else if(m_affectations[1] == nullptr){
        for(m_affectations[0]->executer(); m_condition->executer();){
            m_sequence->executer();
        }
    } else {
        for(m_affectations[0]->executer(); m_condition->executer(); m_affectations[1]->executer()){
            m_sequence->executer();
        }
    }
    return 0;
}

void NoeudInstPour::traduitEnCPP(ostream & cout,unsigned int indentation) const {
    
    if(m_affectations[0] == nullptr && m_affectations[1] == nullptr){
        //cas for(;cond;)
        cout << "\n" << setw(4*indentation) << "" << "for(; ";
        m_condition->traduitEnCPP(cout, 0);
        cout << " ;) {" << endl;
    } else if(m_affectations[0] == nullptr){
        //cas for(;cond;expr)
        cout << "\n" << setw(4*indentation) << "" << "for(; "; 
        m_condition->traduitEnCPP(cout, 0);
        cout << "; "; 
        m_affectations[1]->traduitEnCPP(cout, 0);
        cout << ") {" << endl;
    } else if(m_affectations[1] == nullptr){
        cout << "\n" << setw(4*indentation) << "" << "for(";
        m_affectations[0]->traduitEnCPP(cout, 0);
        cout << "; ";
        m_condition->traduitEnCPP(cout, 0);
        cout << " ;) {" << endl;
    } else {
        cout << "\n" << setw(4*indentation) << "" << "for(";
        m_affectations[0]->traduitEnCPP(cout, 0);
        cout << "; ";
        m_condition->traduitEnCPP(cout, 0);
        cout << "; ";
        m_affectations[1]->traduitEnCPP(cout, 0);
        cout << ") {" << endl;
    }
    m_sequence->traduitEnCPP(cout, indentation+1); // on met la séquence 
    cout << setw(4*indentation)<<""<< "}";
    
 
}

////////////////////////////////////////////////////////////////////////////////
// NoeudEcrire
////////////////////////////////////////////////////////////////////////////////
 
NoeudInstEcrire::NoeudInstEcrire(Noeud* noeudPremierElement, vector<Noeud*> noeudsSupp)
: m_noeud(noeudPremierElement), m_noeudsSupp(noeudsSupp) {
}

int NoeudInstEcrire::executer() {
    Noeud* p;
    p = m_noeud; // on pointe sur le noeud du premier element

    // on regarde si l’objet pointé par p est de type SymboleValue et si c’est une chaîne
    if ((typeid (*p) == typeid (SymboleValue)) && (*((SymboleValue*) p) == "<CHAINE>" )) {
        cout << ((SymboleValue*) p)->getChaine().substr(1, ((SymboleValue*) p)->getChaine().length() - 2) ; //on affiche la chaine de caractere du symbole value de p
    } else {
        cout << p->executer() ; // on affiche le résultat
    }

    for (int i = 0; i < m_noeudsSupp.size(); i++) {
        p = m_noeudsSupp[i]; // on fait pointer p sur l'element courant du vecteur
        if ((typeid (*p) == typeid (SymboleValue)) && (*((SymboleValue*) p) == "<CHAINE>" )) {
            cout << ((SymboleValue*) p)->getChaine().substr(1, ((SymboleValue*) p)->getChaine().length() - 2) ; //on affiche la chaine de caractere
        } else {
            cout << p->executer(); // on affiche le résultat
        }
    }
    cout << endl;
    return 0;
}

void NoeudInstEcrire::traduitEnCPP(ostream & cout,unsigned int indentation) const {
    unsigned int i=0;
    cout <<setw(4*indentation)<<"" <<"cout << " ;
    m_noeud->traduitEnCPP(cout, 0);
    
    while(i<m_noeudsSupp.size()){ // tant qu'il y a des choses à écrire
        cout <<" << ";
        m_noeudsSupp.at(i)->traduitEnCPP(cout, 0);
        i++;
    }
    cout <<setw(4*indentation)<< " << endl;" ;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudLire
////////////////////////////////////////////////////////////////////////////////

NoeudInstLire::NoeudInstLire(Noeud* noeud, vector<Noeud*> noeuds)
: m_noeud(noeud), m_noeudsSupp(noeuds) {
}

int NoeudInstLire::executer() {
    // lire va récuper les valeurs dans la console et va les assigner a ses variables mise en parametres
    Noeud* p;
    p = m_noeud; // on pointe sur le noeud m_noeud
    int varTemp = 0;

    cin >> varTemp; //on dnne une valeur a un entier temporaire via la saisie de l'utilisateur
    ((SymboleValue*) p)->setValeur(varTemp); // on initialise la valeur de la variable avec l'entier temporaire

    for (int i = 0; i < m_noeudsSupp.size(); i++) {
        p = m_noeudsSupp[i]; // on fait pointer p sur l'element courant du vecteur
        cin >> varTemp;
        ((SymboleValue*) p)->setValeur(varTemp);
    }

    return 0;
}

void NoeudInstLire::traduitEnCPP(ostream & cout,unsigned int indentation) const {
    unsigned int i=0;
    cout <<setw(4*indentation)<<""<< "cin >> " ;
    m_noeud->traduitEnCPP(cout, 0); // cin >> variable
    
    while(i<m_noeudsSupp.size()){ // si il y a plusieurs variables
        cout <<" >> ";
        m_noeudsSupp.at(i)->traduitEnCPP(cout, 0);
        i++;
    }
    cout << ";" << endl;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSelon
////////////////////////////////////////////////////////////////////////////////

NoeudInstSelon::NoeudInstSelon(Noeud* variable, Noeud* entier, Noeud* sequence, std::vector<Noeud*> casSupp, std::vector<Noeud*> defaut) 
: m_variable(variable), m_entier(entier), m_sequence(sequence), m_casSupp(casSupp), m_defaut(defaut){
}
//casSupp contient : entier | sequence | entier | sequence | ... | sequence
int NoeudInstSelon::executer() {
    
    bool controleSequence = true;
    int valeur = ((SymboleValue*)m_variable)->getValeur(); // on créé une variable int et on y ajoute la valeur de la variable mise en paramètre
    
    if(valeur == m_entier->executer()){ // si la variable est égale a la valeur du cas
        m_sequence->executer(); // on exécute la séquence
        controleSequence =false; // on sort de la boucle afin qu'aucun autre cas ne soit exécuter
    }
    
    if(!m_casSupp.empty() && controleSequence){
        int i=0; // dans la boucle, i ne sera que sur les entiers, ce qui explique son incrémentation par deux
        while(i<m_casSupp.size() && controleSequence == true){ // tant qu'il y a des cas et que la séquence d'un cas n'a pas déjà été réalisée
        // m_casSupp.at(i)   = entier
        // m_casSupp.at(i+1) = séquence
            if(valeur == m_casSupp.at(i)->executer()){ // si la valeur est égale à l'entier du cas
               m_casSupp.at(i+1)->executer(); // on exécute la séquence
                controleSequence =false; // on sort de la boucle car on a exécuter une séquence
            }
            i+=2; // on passe au prochain entier
        } 
    }
    
    if(!m_defaut.empty() && controleSequence){ // si il a un defaut et qu'aucune sequence n'a été faite
        m_defaut.at(0)->executer(); // on execute la séquence
    }
}
void NoeudInstSelon::traduitEnCPP(ostream& cout, unsigned int indentation) const {
    cout << setw(4*indentation) <<""<<"switch(";
    m_variable->traduitEnCPP(cout, 0); // ecriture de la variable à contrôler
    cout << ") {" << endl;
    
    cout << setw(4*(indentation+1)) <<"" << "case ";
    m_entier->traduitEnCPP(cout, 0); // ecriture du chiffre du case qui correspond à un des cas
    cout << " : " << endl;
    m_sequence->traduitEnCPP(cout, indentation+2);
    cout << "\n"  << setw(4*(indentation+1)) <<"" << "break;";
    
    int i=0;
    while(i<m_casSupp.size()){ // tant qu'il y a des cas (à partir du cas n°2)
        cout << "\n" << setw(4*(indentation+1)) <<"" << "case ";
        m_casSupp.at(i)->traduitEnCPP(cout, 0);
        cout << " : " << endl;
        m_casSupp.at(i+1)->traduitEnCPP(cout, indentation+2);
        cout << "\n"  << setw(4*(indentation+1)) <<"" << "break;";
        i+=2;
    }
    
    if(m_defaut.size()>0){ // si il y a un default (qui correspond à un else)
        cout << "\n" << setw(4*(indentation+1))<<"" << "default : "<< endl;
        m_defaut.at(0)->traduitEnCPP(cout, indentation+2);
    }
    
    cout << setw(4*indentation) <<"" << "}";
}