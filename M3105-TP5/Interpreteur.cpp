#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(nullptr) {
}

void Interpreteur::analyse() {
  m_arbre = programme(); // on lance l'analyse de la première règle
}

void Interpreteur::tester(const string & symboleAttendu) const throw (SyntaxeException) {
  // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
  static char messageWhat[256];
  if (m_lecteur.getSymbole() != symboleAttendu) {
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - Symbole attendu : %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(),
            symboleAttendu.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
  }
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException) {
  // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
  tester(symboleAttendu);
  m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const throw (SyntaxeException) {
  // Lève une exception contenant le message et le symbole courant trouvé
  // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
  static char messageWhat[256];
  sprintf(messageWhat,
          "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
          m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
  throw SyntaxeException(messageWhat);
}

Noeud* Interpreteur::programme() {
  // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
  testerEtAvancer("procedure");
  testerEtAvancer("principale");
  testerEtAvancer("(");
  testerEtAvancer(")");
  Noeud* sequence = seqInst();
  testerEtAvancer("finproc");
  tester("<FINDEFICHIER>");
  return sequence;
}

Noeud* Interpreteur::seqInst() {
  // <seqInst> ::= <inst> { <inst> }
  NoeudSeqInst* sequence = new NoeudSeqInst();
  do {
    sequence->ajoute(inst());
  } while (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "si"
          || m_lecteur.getSymbole() == "tantque" || m_lecteur.getSymbole() == "repeter"
          || m_lecteur.getSymbole() == "pour" || m_lecteur.getSymbole() == "ecrire"
          || m_lecteur.getSymbole() == "lire" || m_lecteur.getSymbole()=="selon");
  // Tant que le symbole courant est un début possible d'instruction...
  // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
  return sequence;
}

Noeud* Interpreteur::inst() {
  // <inst> ::= <affectation>  ; | <instSi> | <instTantQue> 
    try{
        if (m_lecteur.getSymbole() == "<VARIABLE>") {
            Noeud *affect = affectation();
            testerEtAvancer(";");
            return affect;
        }else if (m_lecteur.getSymbole() == "si"){
              return instSiRiche();
        }else if (m_lecteur.getSymbole() == "tantque"){
            return instTantQue();
        }else if (m_lecteur.getSymbole() == "repeter"){
            return instRepeter();
        }else if (m_lecteur.getSymbole() == "pour"){
            return instPour();
        }else if (m_lecteur.getSymbole() == "ecrire"){
            return instEcrire();
        }else if (m_lecteur.getSymbole() == "lire"){
            return instLire();
        }else if (m_lecteur.getSymbole() == "selon"){
            return instSelon();
        }else{
            erreur("Cette instruction est incorrecte");
        }
    }catch(SyntaxeException const& e){ // on récupère l'exception qui a été levée
        cout << e.what() << endl;
        while((m_lecteur.getSymbole()!="si"&& m_lecteur.getSymbole()!="tantque" && m_lecteur.getSymbole()!="pour" &&
               m_lecteur.getSymbole()!="ecrire" && m_lecteur.getSymbole()!="lire") && m_lecteur.getSymbole()!="<FINDEFICHIER>"){
            m_lecteur.avancer(); // on fait avancer le lecteur tant qu'il ne lit pas un des symbole du while
        }
    }
}

Noeud* Interpreteur::affectation() {
  // <affectation> ::= <variable> = <expression> 
  tester("<VARIABLE>");
  Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table et on la mémorise
  m_lecteur.avancer();
  testerEtAvancer("=");
  Noeud* exp = expression();             // On mémorise l'expression trouvée
  return new NoeudAffectation(var, exp); // On renvoie un noeud affectation
}

Noeud* Interpreteur::expression() {
  // <expression> ::= <facteur> { <opBinaire> <facteur> }
  //  <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
  Noeud* term = terme();
  while ( m_lecteur.getSymbole() == "+" || m_lecteur.getSymbole() == "-") {
    Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
    m_lecteur.avancer();
    Noeud* factDroit = terme(); // On mémorise l'opérande droit
    term = new NoeudOperateurBinaire(operateur, term, factDroit); // Et on construuit un noeud opérateur binaire
  }
  return term; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::terme(){
    Noeud* fact = facteur();
    while (m_lecteur.getSymbole() == "*" || m_lecteur.getSymbole() == "/") {
        Symbole operateur = m_lecteur.getSymbole();
        m_lecteur.avancer();
        Noeud* factDroit = facteur();
        fact = new NoeudOperateurBinaire(operateur, fact, factDroit);
    }
    return fact;
}

Noeud* Interpreteur::facteur() {
   // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
   Noeud* fact = nullptr;
   if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
     fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
     m_lecteur.avancer();
  } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
    m_lecteur.avancer();
    // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
    fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), expBool());
  } else if (m_lecteur.getSymbole() == "non") { // non <facteur>
    m_lecteur.avancer();
    // on représente le moins unaire (- facteur) par une soustractin binaire (0 - facteur)
    fact = new NoeudOperateurBinaire(Symbole("non"), expBool(), nullptr);
  } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
    m_lecteur.avancer();
    fact = expBool();
    testerEtAvancer(")");
  } else
    erreur("Facteur incorrect");
  return fact;
}

Noeud* Interpreteur::expBool() {
    Noeud* relGauche = relationET();
    while (m_lecteur.getSymbole() == "ou") {
        Symbole operateur = m_lecteur.getSymbole();
        m_lecteur.avancer();
        Noeud* relDroit = relationET();
        relGauche = new NoeudOperateurBinaire(operateur,relGauche,relDroit);
    }
    return relGauche;
}
Noeud* Interpreteur::relationET() {
    Noeud* relETGauche = relation();
    while (m_lecteur.getSymbole() == "et") {
        Symbole operateur = m_lecteur.getSymbole();
        m_lecteur.avancer();
        Noeud* relETDroit = relation();
        relETGauche = new NoeudOperateurBinaire(operateur,relETGauche,relETDroit);
    }
    return relETGauche;
}
Noeud* Interpreteur::relation() {
    Noeud* expGauche = expression();
    while(m_lecteur.getSymbole() == "<" || m_lecteur.getSymbole() == "<=" ||
            m_lecteur.getSymbole() == ">" || m_lecteur.getSymbole() == ">=" ||
            m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!="){
        Symbole operateur = m_lecteur.getSymbole();
        m_lecteur.avancer();
        Noeud* expDroit = expression();
        expGauche = new NoeudOperateurBinaire(operateur,expGauche,expDroit);
    }
    return expGauche;
}

Noeud* Interpreteur::instSi() {
  // <instSi> ::= si ( <expression> ) <seqInst> finsi
  testerEtAvancer("si");
  testerEtAvancer("(");
  Noeud* condition = relation(); // On mémorise la condition
  testerEtAvancer(")");
  Noeud* sequence = seqInst();     // On mémorise la séquence d'instruction
  testerEtAvancer("finsi");
  return new NoeudInstSi(condition, sequence); // Et on renvoie un noeud Instruction Si
}

Noeud* Interpreteur::instSiRiche() {
  // <instSiRiche> ::= si(<expression>) <seqInst> {sinonsi(<expression>) <seqInst> }[sinon <seqInst>]finsi
  vector<Noeud*> conditions;
  vector<Noeud*> sequences;
  testerEtAvancer("si");
  testerEtAvancer("(");
  conditions.push_back(relation()); // On mémorise la condition
  testerEtAvancer(")");
  sequences.push_back(seqInst()); // On mémorise la séquence d'instruction
  while(m_lecteur.getSymbole() == "sinonsi") {
    testerEtAvancer("sinonsi");
    testerEtAvancer("(");
    conditions.push_back(relation()); // On mémorise la condition
    testerEtAvancer(")");
    sequences.push_back(seqInst()); // On mémorise la séquence d'instruction
  }
  if(m_lecteur.getSymbole() == "sinon") {
     testerEtAvancer("sinon");
     sequences.push_back(seqInst()); // On mémorise la séquence d'instruction
  }
  testerEtAvancer("finsi");
  return new NoeudInstSiRiche(conditions, sequences);
}

Noeud* Interpreteur::instTantQue() {
    // <instTantQue> ::= { <expression> } <seqInst> fintantque
    testerEtAvancer("tantque");
    testerEtAvancer("(");
    Noeud* condition = relation(); // On mémorise la condition
    testerEtAvancer(")");
    Noeud* sequence = seqInst();     // On mémorise la séquence d'instruction
    testerEtAvancer("fintantque");
    return new NoeudInstTantQue(condition, sequence);    
}

Noeud*  Interpreteur::instRepeter(){
    // <instRepeter> ::= repeter <sequInst> jusqua (<expression>)
    testerEtAvancer("repeter");
    Noeud* sequence = seqInst();     // On mémorise la séquence d'instruction
    testerEtAvancer("jusqua");
    testerEtAvancer("(");
    Noeud* condition = relation(); // On mémorise la condition
    testerEtAvancer(")");
    return new NoeudInstRepeter(condition, sequence);
}

Noeud* Interpreteur::instPour(){
    // <instPour> ::=  pour ( [ <affectation> ]  ; <expression> ; [ <affectation> ] ) <seqInst> fin pour
    testerEtAvancer("pour");
    testerEtAvancer("(");
    vector<Noeud*> affectations;
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        affectations.insert(affectations.begin(), affectation());
    } else {
        affectations.insert(affectations.begin(), nullptr);
    }
    testerEtAvancer(";");
    Noeud* condition = relation();
    testerEtAvancer(";");
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        affectations.insert(affectations.begin()+1, affectation());
    } else {
        affectations.insert(affectations.begin()+1, nullptr);
    }
    testerEtAvancer(")");
    Noeud* sequence = seqInst();
    testerEtAvancer("finpour");
    return new NoeudInstPour(affectations, condition, sequence);    
}

Noeud* Interpreteur::instEcrire() {
    // <instEcrire>  ::= ecrire( <expression> | <chaine> {, <expression> | <chaine> })
    Noeud* noeud = nullptr;
    Noeud* noeud2 = nullptr;
    testerEtAvancer("ecrire");
    testerEtAvancer("(");

    if (m_lecteur.getSymbole() == "<CHAINE>") {
        noeud = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
        m_lecteur.avancer();
        
    }else{ // sinon c'est une expression
        noeud = relation();
    }
    
    vector<Noeud*> noeudsSupp;
    
    while(m_lecteur.getSymbole()==","){ // on regarde si il y a d'autres choses à écrire
        testerEtAvancer(",");
        if (m_lecteur.getSymbole() == "<CHAINE>") {
            noeud2 = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
            m_lecteur.avancer();
            noeudsSupp.push_back(noeud2);
        }else { // si le symbole lu est un entier , ça veut dire que c'est une expression
            noeud2 = relation();
            noeudsSupp.push_back(noeud2);
        }
    }
    testerEtAvancer(")");
    
    return new NoeudInstEcrire(noeud,noeudsSupp); // on retourn un noeud inst Ecrire
}

Noeud* Interpreteur::instLire(){
    // <instLire>    ::= lire ( <variable> {, <variable> })
    Noeud* noeud = nullptr;
    Noeud* noeud2 =nullptr;
    vector<Noeud*> noeudsLireSupp;
    testerEtAvancer("lire");
    testerEtAvancer("(");
    if (m_lecteur.getSymbole() == "<VARIABLE>"){ // si lecteur lit bien un sylbole de type VARIABLE
        noeud = m_table.chercheAjoute(m_lecteur.getSymbole());//on ajoute la variable à la table de symbole values
        m_lecteur.avancer();
    }
    
    while (m_lecteur.getSymbole()==","){ // tant qu'il y a de variables à lire
        testerEtAvancer(",");
        if (m_lecteur.getSymbole() == "<VARIABLE>"){ // si lecteur lit bien un sylbole de type VARIABLE
            noeud2 = m_table.chercheAjoute(m_lecteur.getSymbole());//on ajoute la variable à la table de symbole values
            noeudsLireSupp.push_back(noeud2);
            m_lecteur.avancer();
        }
    }
    testerEtAvancer(")");
    
    //return nullptr;
    return new NoeudInstLire(noeud,noeudsLireSupp);
}

void Interpreteur::traduitEnCPP(ostream & cout,unsigned int indentation)const{
    cout << "\n\n// ===================TRADUCTION EN C++================\n"<<endl;
    cout << setw(4*indentation)<<""<<"#include <iostream>"<<endl; // affiche tout le nécessaire pour que le code généré puisse compiler
    cout << setw(4*indentation)<<""<<"using namespace std;"<<endl;
    cout << setw(4*indentation)<<""<<"\nint main() {"<< endl;
  
  for (int i = 0 ; i < m_table.getTaille() ; i++){ // pour écrire int i; int j; etc...
      if (m_table[i]=="<VARIABLE>"){
          cout << setw(4* (indentation+1)) << "" << "int " << m_table[i].getChaine()<<";"<<endl;
      }
  }
  getArbre()->traduitEnCPP(cout,indentation+1);// lance l'opération traduitEnCPP sur la racine
  cout <<"\n"<< setw(4*(indentation+1))<<""<<"return 0;"<< endl ; 
  cout << setw(4*indentation)<<"}" << endl ; // Fin d’un programme C++
}

Noeud* Interpreteur::instSelon() {
    // <instSelon>   ::= selon( <variable>) cas <entier> : <seqInst>  {cas <entier>: <seqInst> } [defaut : <seqInst>] finselon
    
    testerEtAvancer("selon");
    testerEtAvancer("(");
    Noeud* variable = m_table.chercheAjoute(m_lecteur.getSymbole());//on ajoute la variable à la table de symbole values
    m_lecteur.avancer();
    testerEtAvancer(")");
    testerEtAvancer("cas");
    Noeud* entier = m_table.chercheAjoute(m_lecteur.getSymbole());// on ajoute l'entier à la table des symboles
    m_lecteur.avancer();
    testerEtAvancer(":");
    Noeud* sequence = seqInst();
    
    vector<Noeud*> casSupp;
    vector<Noeud*> defaut;
    Noeud* entier2;
    Noeud* sequence2;
    
    while(m_lecteur.getSymbole()== "cas"){ // tant qu'il y a des cas
        testerEtAvancer("cas");
        entier2 = m_table.chercheAjoute(m_lecteur.getSymbole());
        m_lecteur.avancer();
        casSupp.push_back(entier2);
        testerEtAvancer(":");
        sequence2 = seqInst(); // on créer la séquence
        casSupp.push_back(sequence2);
    }
    
    if (m_lecteur.getSymbole()=="defaut"){ // si le cas par défaut à été renseigné
        testerEtAvancer("defaut");
        testerEtAvancer(":");
        Noeud* sequenceDefaut = seqInst();
        defaut.push_back(sequenceDefaut); // on ajoute la séquence au vecteur du sinon
    }
    
    testerEtAvancer("finselon");
    return new NoeudInstSelon(variable,entier,sequence,casSupp,defaut);
    
}


