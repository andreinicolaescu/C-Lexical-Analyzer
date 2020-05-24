/*

	Nume : Nicolaescu Andrei - George
	Grupa : 342
	Semigrupa : 1

	Programul de fata este un analizor lexical, scris in C++, pentru C++.
	Foloseste doua clase : Token si DFA.

	__CLASA TOKEN__ :

		- reprezinta un model pentru fiecare cuvant ce are valoare in limbajul tinta (cuvinte cheie, identificatori, operatori etc.);
		- are doua campuri : valoare (sirul de caractere efectiv al token-ului) si tip (categoria in care se incadreaza token-ul);

	__CLASA DFA__ :

		- reprezinta un automat finit determinist, cu care am modelat analizorul;
		- are tranzitii valide care sunt definite, iar daca intalnim o tranzitie nedefinita (invalida), se arunca o eroare si analizorul se opreste;
		- functia principala 'getNextToken' primeste numele unui fisier pe care-l citeste intr-un buffer si identifica token-urile;
		- automatul porneste cu starea Start, care este starea initiala, si parseaza caracterele din fisierul tinta, realizand tranzitii;
		- pe masura ce token-urile sunt identificate, automatul printeaza;
		- atunci cand apare o eroare, automatul se opreste;


	__TRANZITII POSIBILE__ :

		START ---( cifra )---> CIFRE
		START ---( . si cifra )---> FLOAT
		START ---( litera )---> IDENTIFICATOR
		START ---( _ )---> IDENTIFICATOR
		START ---( / * )---> PARSING COMENTARIU MULTIPLU
		START ---( // )---> COMENTARIU SIMPLU
		START ---( / )---> OPERATOR
		START ---( " )---> PARSING LITERAL STRING GHILIMELE
		START ---( ' )---> PARSING LITERAL STRING APOSTROF
		START ---( separator valid )---> SEPARATOR FINAL
		START ---( operator )---> OPERATOR
		START ---( orice whitespace )---> START

		CIFRE ---( cifra )---> CIFRE
		CIFRE ---( . )---> FLOAT
		CIFRE ---( e )---> EXPONENTIAL
		CIFRE ---( orice whitespace )---> CIFRE FINAL

		FLOAT ---( cifra )---> FLOAT
		FLOAT ---( e )---> EXPONENTIAL

		EXPONENTIAL ---( + si cifra)---> EXPONENTIAL
		EXPONENTIAL ---( - si cifra)---> EXPONENTIAL
		EXPONENTIAL ---( cifra )---> EXPONENTIAL

		PARSING LITERAL STRING GHILIMELE ---( orice caracter )---> PARSING LITERAL STRING GHILIMELE
		PARSING LITERAL STRING GHILIMELE ---( " )---> LITERAL STRING FINAL

		PARSING LITERAL STRING APOSTROF ---( orice caracter )---> PARSING LITERAL STRING APOSTROF
		PARSING LITERAL STRING APOSTROF ---( ' )---> LITERAL STRING FINAL

		COMENTARIU SIMPLU ---( orice caracter )---> COMENTARIU SIMPLU
		COMENTARIU SIMPLU ---( newline )---> COMENTARIU SIMPLU FINAL

		PARSING COMENTARIU MULTIPLU ---(orice caracter)---> PARSING COMENTARIU MULTIPLU
		PARSING COMENTARIU MULTIPLU ---( '* /' )--->COMENTARIU MULTIPLU FINAL

		IDENTIFICATOR ---( litera )---> IDENTIFICATOR
		IDENTIFICATOR ---( _ )---> IDENTIFICATOR
		IDENTIFICATOR ---( verificare )---> KEYWORD FINAL, BOOLEAN FINAL, OPERATOR SPECIAL FINAL   // (vezi functii DFA)

		OPERATOR ---( verificare )---> OPERATOR (cu un caracter) FINAL
		OPERATOR ---( verificare )---> OPERATOR (cu doua caractere) FINAL
*/


#include <iostream>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

class Token {

	int index;
	string type;

public:

	Token();

	void setValue(int index); //setteri pentru valoare si tip
	void setType(string type);

	int getValue() {
		return this->index;
	}

	string getType() {
		return this->type;
	}

};

void Token::setValue(int index) {
	this->index = index;
}

void Token::setType(string type) {
	this->type = type;
}

Token::Token() {
	index = 0;
	type = "";
}

class DFA {

	string currentState; //starea curenta in care se gaseste automatul

public:
	DFA();

	string buffer; //buffer cu care parsez pana identific un token
	Token currentToken; //token-ul curent
	vector<pair <string, string> > uniqueStrings;

	bool scanToken(const string& nameOfFile, unsigned int& crtIndex);
	Token getNextToken(const string& nameOfFile, unsigned int& crtIndex); //functia principala
	void printToken(Token crtToken); //afiseaza token-ul identificat

	int addToUniques(string buffer);

	string getCurrentState(); //returneaza starea curenta
	void changeState(string destination); //realizeaza o tranzitie catre o stare 'destinatie'

	void reset(); //reseteaza starea la cea 'Start' (de parsare/asteptare)
	void error(string& buffer); //starea de 'eroare'

	bool checkCifra(char& ch); //functie ce verifica daca un caracter este o cifra
	bool checkLitera(char& ch); //functie ce verifica daca un caracter este o litera
	bool checkHexa(char& ch);
	bool checkOcta(char& ch);
	bool checkInvalidOcta(char& ch);

	// functii ce verifica diferite tipuri de token //
	bool checkKeyword(string& buffer);
	bool checkSpecialOperator(string& buffer);
	bool checkSeparator(char& buffer);
	bool checkOperator1(char& buffer);
	bool checkOperator2(string& buffer);
};

DFA::DFA() {
	buffer = "";
	Token currentToken;
	this->currentState = "Start";
}

string DFA::getCurrentState() {
	return this->currentState;
}

void DFA::changeState(string destination) {
	this->currentState = destination;
}

void DFA::error(string& buffer)
{
	cout << "\nEROARE intalnita la parsarea tokenului : " << buffer << "\n---------------------------\n";
}

void DFA::printToken(Token crtToken)
{
	if (crtToken.getType() == "ERROR")
	{
		cout << "\nEROARE intalnita la parsarea tokenului : " << uniqueStrings[crtToken.getValue()].first << "\n---------------------------\n";
		exit(666);
	}
	else {
		cout << uniqueStrings[crtToken.getValue()].first << "  \t\t   ----> Token gasit [ " << crtToken.getType() << " ]\n_________________\n";
	}
	this->currentToken.setType("");
	this->currentToken.setValue(-1);
}

int DFA::addToUniques(string buffer)
{
	bool found = false;
	int index;
	for (unsigned int i = 0; i < uniqueStrings.size(); i++)
	{
		if (uniqueStrings[i].first == buffer)
		{
			found = true;
			index = i;
		}
	}
	if (!found)
	{
		index = uniqueStrings.size();
		pair<string, string> entry = make_pair(buffer, currentToken.getType());
		uniqueStrings.push_back(entry);

	}

	return index;
}

bool DFA::checkCifra(char& ch) {
	return (int(ch) >= 48 && int(ch) <= 57);
}

bool DFA::checkLitera(char& ch) {
	return (int(ch) >= 65 && int(ch) <= 90) || (int(ch) >= 97 && int(ch) <= 122);
}

bool DFA::checkHexa(char& ch) {
	return (int(ch) >= 48 && int(ch) <= 57) || (int(ch) >= 65 && int(ch) <= 70) || (int(ch) >= 97 && int(ch) <= 102);
}

bool DFA::checkOcta(char& ch) {
	return (int(ch) >= 48 && int(ch) <= 55);
}

bool DFA::checkInvalidOcta(char& ch) {
	return ch == '8' || ch == '9';
}

bool DFA::checkKeyword(string& buffer)
{
	static const string arr[] = { "auto","break","case","char","const","continue","start",
								"do","double","else","enum","extern","float","for","goto",
								"if","int","long","register","return","short","signed",
								"sizeof","static","struct","switch","typedef","union",
								"unsigned","void","volatile","while", "delete", "throw", };

	vector<string> keywords(arr, arr + sizeof(arr) / sizeof(arr[0]));

	for (unsigned int i = 0; i < keywords.size(); i++)
		if (keywords[i] == buffer)
			return true;

	return false;

}

bool DFA::checkSeparator(char& buffer)
{
	const string ch = string(1, buffer);

	static const string arr[] = { "{", "}", ",", "(", ")", ";" , "[", "]"};
	vector<string> separators(arr, arr + sizeof(arr) / sizeof(arr[0]));

	for (unsigned int i = 0; i < separators.size(); i++)
		if (ch == separators[i])
			return true;

	return false;
}

bool DFA::checkSpecialOperator(string& buffer)
{
	static const string arr[] = { "mod", "div", "new", "delete" };
	vector<string> operators(arr, arr + sizeof(arr) / sizeof(arr[0]));

	for (unsigned int i = 0; i < operators.size(); i++)
		if (operators[i] == buffer)
			return true;

	return false;
}

bool DFA::checkOperator1(char& buffer)
{

	const string ch = string(1, buffer);

	static const string arr[] = { "<", ">", "*", "+", "-", "%","/", "=", ".", "&", "|", "~", "^", "!", ":", "?"};
	vector<string> operators(arr, arr + sizeof(arr) / sizeof(arr[0]));

	for (unsigned int i = 0; i < operators.size(); i++)
		if (operators[i] == ch)
			return true;

	return false;
}

bool DFA::checkOperator2(string& buffer)
{

	static const string arr[] = {"<=", ">=","!=", "-=", "*=", "+=", "%=", "/=", "++", "--", "==", "||", "&&", "<<", ">>", "->", "^=", "&=", "|="};
	vector<string> operators(arr, arr + sizeof(arr) / sizeof(arr[0]));

	for (unsigned int i = 0; i < operators.size(); i++)
		if (operators[i] == buffer)
			return true;

	return false;
}

void DFA::reset()
{
	changeState("Start");
	buffer = "";
}

bool DFA::scanToken(const string& nameOfFile, unsigned int& crtIndex)
{
	ifstream inFile;
	inFile.open(nameOfFile.c_str()); // Deschidem fisierul de input

	stringstream strStream;
	strStream << inFile.rdbuf(); // Citim fisierul cu ajutorul unui flux de tip string
	string inputFile = strStream.str(); // Stocam continutul in variabila 'inputFile'

	//unsigned int crtIndex = 0; // Indexul curent la care ne aflam in fisierul input
	char currentCh; // O variabila ce tine caracterul curent

	while (crtIndex <= inputFile.length()) // Cat timp citim din fisier...
	{

		currentCh = inputFile[crtIndex];

		if (isspace(currentCh) && getCurrentState() == "Start") {
			// Sar peste spatiile goale daca ma aflu in starea Start.
		}
		else if (getCurrentState() == "Start") // Daca ma situez in starea Start...
		{
			if (currentCh == '0')
			{
				if (inputFile[crtIndex + 1] == 'x' || inputFile[crtIndex + 1] == 'X')
				{
					changeState("Hexa");
					buffer += currentCh;
					buffer += inputFile[crtIndex + 1];
					crtIndex++;
				}
				else if (checkOcta(inputFile[crtIndex + 1]))
				{
					changeState("Octa");
					buffer += currentCh;
				}
				else
				{
					changeState("Cifre");
					buffer += currentCh;
				}
			}
			else if (checkCifra(currentCh)) // ...si intalnesc o cifra, merg in starea Cifre.
			{
				changeState("Cifre");
				buffer += currentCh;
			}
			else if (currentCh == '.' && checkCifra(inputFile[crtIndex + 1])) // ...si intalnesc un punct urmat de cifra, merg in starea Float.
			{
				changeState("Float");
				buffer += currentCh;
			}
			else if (checkLitera(currentCh) || currentCh == '_') // ...si intalnesc litere sau '_', merg in starea Identificator.
			{
				changeState("Identificator");
				buffer += currentCh;
			}
			else if (currentCh == '/') // ... si intalnesc caracterul '/', verific ce urmeaza dupa acesta.
			{
				if (inputFile[crtIndex + 1] == '*') // Daca urmeaza '*', se realizeaza tranzitie in starea de asteptare a comentariului multiplu.
				{
					changeState("ParsingComentariuMultiplu");
					buffer += currentCh;
					crtIndex++;
					buffer += inputFile[crtIndex + 1];
				}
				else if (inputFile[crtIndex + 1] == '/') // Daca urmeaza inca un caracter '/', atunci intru in starea de comentariu simplu.
				{
					changeState("ComentariuSimplu");
					buffer += currentCh;
					crtIndex++;
					buffer += inputFile[crtIndex + 1];
				}
				else // Altfel, este clar ca am intalnit operatorul '/'.
				{
					changeState("Operator");
					buffer += currentCh;
				}
			}
			else if (currentCh == '"') // Intru in starea de asteptare a literalului string cu ghilimele.
			{
				changeState("ParsingLiteralString1");
				buffer += currentCh;
			}
			else if (currentCh == '\'') // Intru in starea de asteptare a literalului string cu apostrof.
			{
				changeState("ParsingLiteralString2");
				buffer += currentCh;
			}
			else if (checkOperator1(currentCh)) // Daca caracterul curent este un operator valid, intru in starea operator.
			{
				changeState("Operator");
				buffer += currentCh;
			}
			else if (checkSeparator(currentCh)) // Daca caracterul curent este un separator valid, intru in starea separator.
			{
				changeState("Separator");
				buffer += currentCh;
				reset();
				crtIndex++;
				return true;
			}
			else if (currentCh == ' ') // Daca intalnim spatiu, continuam.
			{
				continue;
			}

		}
		else if (getCurrentState() == "Unsigned")
		{
			if (currentCh == 'l' || currentCh == 'L')
			{
				buffer += currentCh;
				changeState("LiteralUnsignedLong");

				if (inputFile[crtIndex + 1] == 'l' || inputFile[crtIndex + 1] == 'L' || inputFile[crtIndex + 1] == 'u' || inputFile[crtIndex + 1] == 'U')
				{
					buffer += inputFile[crtIndex + 1];
					crtIndex++;
					//error(buffer);
					//exit(666);
					reset();
					return true;
				}
				else if (checkLitera(inputFile[crtIndex + 1]) || inputFile[crtIndex + 1] == '_' || checkCifra(inputFile[crtIndex + 1])) // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
				{
					buffer += inputFile[crtIndex + 1];
					crtIndex++;
					//error(buffer);
					//exit(666);
					reset();
					return true;
				}
				else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
				{
					reset();
					return true;
					crtIndex--;
				}
			}
			else if (checkLitera(currentCh) || currentCh == '_' || checkCifra(currentCh)) // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
			{
				buffer += currentCh;
				//error(buffer);
				//exit(666);
				reset();
				return true;
			}
			else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
			{
				reset();
				return true;
				crtIndex--;
			}
		}
		else if (getCurrentState() == "Long")
		{
			if (currentCh == 'u' || currentCh == 'U')
			{
				buffer += currentCh;
				changeState("LiteralUnsignedLong");

				if (inputFile[crtIndex + 1] == 'l' || inputFile[crtIndex + 1] == 'L' || inputFile[crtIndex + 1] == 'u' || inputFile[crtIndex + 1] == 'U')
				{
					buffer += inputFile[crtIndex + 1];
					crtIndex++;
					//error(buffer);
					//exit(666);
					reset();
					return true;
				}
				else if (checkLitera(inputFile[crtIndex + 1]) || inputFile[crtIndex + 1] == '_' || checkCifra(inputFile[crtIndex + 1])) // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
				{
					buffer += inputFile[crtIndex + 1];
					crtIndex++;
					//error(buffer);
					//exit(666);
					reset();
					return true;
				}
				else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
				{
					reset();
					return true;
					crtIndex--;
				}
			}
			else if (checkLitera(currentCh) || currentCh == '_' || checkCifra(currentCh)) // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
			{
				buffer += currentCh;
				//error(buffer);
				//exit(666);
				reset();
				return true;
			}
			else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
			{
				reset();
				return true;
				crtIndex--;
			}
		}
		else if (getCurrentState() == "Hexa")
		{
			if (checkHexa(currentCh))
			{
				buffer += currentCh;
			}
			else if (currentCh == ' ') // ... si citim spatiu, atunci finalizam cu acest token.
			{
				reset();
				return true;
			}
			else if (currentCh == 'u' || currentCh == 'U')
			{
				changeState("Unsigned");
				buffer += currentCh;
			}
			else if (currentCh == 'l' || currentCh == 'L')
			{
				changeState("Long");
				buffer += currentCh;
			}
			else if (!checkHexa(currentCh) && (checkLitera(currentCh) || currentCh == '_')) // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
			{
				buffer += currentCh;
				//error(buffer);
				//exit(666);
				reset();
				return true;
			}
			else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
			{
				reset();
				return true;
				crtIndex--;
			}
		}
		else if (getCurrentState() == "Octa")
		{
			if (checkOcta(currentCh))
			{
				buffer += currentCh;
			}
			else if (currentCh == ' ') // ... si citim spatiu, atunci finalizam cu acest token.
			{
				reset();
				return true;
			}
			else if (currentCh == 'u' || currentCh == 'U')
			{
				changeState("Unsigned");
				buffer += currentCh;
			}
			else if (currentCh == 'l' || currentCh == 'L')
			{
				changeState("Long");
				buffer += currentCh;
			}
			else if (!checkOcta(currentCh) && (checkLitera(currentCh) || currentCh == '_')) // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
			{
				buffer += currentCh;
				//error(buffer);
				//exit(666);
				reset();
				return true;
			}
			else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
			{
				reset();
				return true;
				crtIndex--;
			}
		}
		else if (getCurrentState() == "Cifre") // Suntem in starea Cifre...
		{
			if (currentCh == '.') // ... si citim un punct, trecem in starea Float.
			{
				changeState("Float");
				buffer += currentCh;
			}
			else if (checkCifra(currentCh)) // ... si citim o cifra, ramanem in aceeasi stare;
			{
				buffer += currentCh;
			}
			else if (currentCh == 'e' || currentCh == 'E') // ... si citim un 'e', trecem in starea de Exponential
			{
				changeState("Exponential");
				buffer += currentCh;
			}
			else if (currentCh == ' ') // ... si citim spatiu, atunci finalizam cu acest token.
			{
				reset();
				return true;
			}
			else if (currentCh == 'u' || currentCh == 'U')
			{
				changeState("Unsigned");
				buffer += currentCh;
			}
			else if (currentCh == 'l' || currentCh == 'L')
			{
				changeState("Long");
				buffer += currentCh;
			}
			else if (checkLitera(currentCh) || currentCh == '_') // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
			{
				buffer += currentCh;
				//error(buffer);
				//exit(666);
				reset();
				return true;
			}
			else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
			{
				reset();
				return true;
				crtIndex--;
			}
		}
		else if (getCurrentState() == "Float") // Din starea Float...
		{
			if (checkCifra(currentCh)) // ... cu cifra ramanem in Float.
			{
				buffer += currentCh;
			}
			else if (currentCh == 'e' || currentCh == 'E') // ... cu 'e' mergem in Exponential.
			{
				changeState("Exponential");
				buffer += currentCh;
			}
			else if (currentCh == 'u' || currentCh == 'U')
			{
				changeState("Unsigned");
				buffer += currentCh;
			}
			else if (currentCh == 'l' || currentCh == 'L')
			{
				changeState("Long");
				buffer += currentCh;
			}
			else if (checkLitera(currentCh) || currentCh == '_') // TRANZITIE INVALIDA DIN FLOAT CU LITERA SAU '_'
			{
				buffer += currentCh;
				reset();
				return true;
				//error(buffer);
				//exit(666);
			}
			else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
			{
				reset();
				return true;
				crtIndex--;
			}
		}
		else if (getCurrentState() == "Exponential") // Din starea Exponential...
		{
			if (currentCh == '-' || currentCh == '+' || checkCifra(currentCh)) // ... cu '+' sau '-' sau orice cifra, ramanem in aceeasi stare.
			{
				buffer += currentCh;
				crtIndex++;
				while (checkCifra(inputFile[crtIndex])) // Cat timp intalnesc cifre, le adaug in buffer.
				{
					buffer += inputFile[crtIndex];
					crtIndex++;
				}
				if (inputFile[crtIndex] == 'u' || inputFile[crtIndex] == 'U')
				{
					changeState("Unsigned");
					buffer += inputFile[crtIndex];
				}
				else if (inputFile[crtIndex] == 'l' || inputFile[crtIndex] == 'L')
				{
					changeState("Long");
					buffer += inputFile[crtIndex];
				}
				else if (checkLitera(inputFile[crtIndex]) || inputFile[crtIndex] == '_')
				{
					// Daca dupa cifrele citite am intalnit o litera sau '_' ...
					// Verific caracterul curent, iar daca acesta ma duce spre un identificator, este invalidata tranzitia.

					buffer += inputFile[crtIndex];
					reset();
					return true;
					//error(buffer);
					//exit(666);
				}
				else // Altfel, printez numarul rezultat si trec mai departe, excluzand caracterul curent parsat deja in verificare.
				{
					reset();
					return true;
					crtIndex--;
				}
			}
			else if (currentCh == 'u' || currentCh == 'U')
			{
				changeState("Unsigned");
				buffer += currentCh;
			}
			else if (currentCh == 'l' || currentCh == 'L')
			{
				changeState("Long");
				buffer += currentCh;
			}
			else if (checkLitera(currentCh) || currentCh == '_') // TRANZITIE INVALIDA DIN FLOAT CU LITERA SAU '_'
			{
				buffer += currentCh;
				reset();
				return true;
				//error(buffer);
				//exit(666);
			}
		}
		else if (getCurrentState() == "ParsingLiteralString1") // Sunt in asteptarea inchiderii unui literal string.
		{
			if (currentCh != '"' && currentCh != '\n') // Cat timp citesc caracterele string-ului, le adaug in buffer.
			{
				if (currentCh == '\\')
				{
					if (inputFile[crtIndex + 1] == '\r' && inputFile[crtIndex + 2] == '\n') // Linux coding
					{
						buffer += currentCh;
						buffer += inputFile[crtIndex + 1];
						buffer += inputFile[crtIndex + 2];
						crtIndex++;
						crtIndex++;
					}
					else if ((inputFile[crtIndex + 1] == '\n') || (inputFile[crtIndex + 1] == '\r')) // Windows & Mac
					{
						buffer += currentCh;
						buffer += inputFile[crtIndex + 1];
						crtIndex++;
					}
					else if (inputFile[crtIndex + 1] == '\"')
					{
						buffer += currentCh;
						buffer += inputFile[crtIndex + 1];
						crtIndex++;
					}
					else {
						buffer += currentCh;
					}
				}
				else {
					buffer += currentCh;
				}
			}
			else if (currentCh == '\n')
			{
				buffer += currentCh;
				reset();
				return true;
				//error(buffer);
				//exit(666);
			}
			else // Am intalnit sfarsitul literalului.
			{
				buffer += currentCh;
				reset();
				crtIndex++;
				return true;
			}
		}
		else if (getCurrentState() == "ParsingLiteralString2") // Sunt in asteptarea inchiderii unui literal string.
		{
			if (currentCh != '\'' && currentCh != '\n') // Cat timp citesc caracterele string-ului, le adaug in buffer.
			{
				if (currentCh == '\\')
				{
					if (inputFile[crtIndex + 1] == '\'')
					{
						buffer += currentCh;
						buffer += inputFile[crtIndex + 1];
						crtIndex++;
					}
					else {
						buffer += currentCh;
					}
				}
				else {
					buffer += currentCh;
				}
			}
			else if (currentCh == '\n')
			{
				buffer += currentCh;
				reset();
				return true;
				//error(buffer);
				//exit(666);
			}
			else // Am intalnit sfarsitul literalului.
			{
				buffer += currentCh;
				reset();
				crtIndex++;
				return true;
			}
		}
		else if (getCurrentState() == "ComentariuSimplu") // Sunt intr-un comentariu simplu.
		{
			if (currentCh == '\n') // Cand ajung la finalul liniei curente, finalizez token-ul.
			{
				reset();
				//return true;
			}
			else // Altfel adaug caracterele in buffer.
			{
				buffer += currentCh;
			}
		}
		else if (getCurrentState() == "ParsingComentariuMultiplu") // Parsez  un comentariu multiplu.
		{
			if (currentCh == '*' && inputFile[crtIndex + 1] == '/') // Daca ii intalnesc sfarsitul, adaug '*/' gasite si finalizez.
			{
				buffer += currentCh;
				buffer += inputFile[crtIndex + 1];
				crtIndex++;
				//crtIndex++;

				reset();
				//return true;
			}
			else // Altfel continui sa adaug pana la incheierea comentariului, ramanand tot in starea de asteptare.
			{
				buffer += currentCh;
			}
		}
		else if (getCurrentState() == "Identificator") // Daca sunt in starea de identificator...
		{
			if (checkLitera(currentCh) || currentCh == '_' || checkCifra(currentCh)) // cu litera sau '_', raman in aceeasi stare.
			{
				buffer += currentCh;
			}
			else // Am intalnit un caracter ce nu face parte din identificator, deci verific tipul token-ului curent si finalizez, excluzand caracterul curent.
			{
				if (checkKeyword(buffer))
				{
					changeState("Keyword");
				}
				else if (buffer == "true" || buffer == "false")
				{
					changeState("Boolean");
				}
				else if (checkSpecialOperator(buffer))
				{
					changeState("Operator");
				}
				reset();
				return true;
				crtIndex--;
			}
		}
		else if (getCurrentState() == "Operator") // Sunt in starea de operator...
		{

			if (checkOperator1(currentCh)) // daca am mai citit inca un operator...
			{

				string copybuffer1 = buffer;
				string check = copybuffer1 += currentCh;
				string checkspecial = copybuffer1 += inputFile[crtIndex+1];
				if (checkspecial == "<<=" || checkspecial == ">>=")
				{
					reset();
					crtIndex++;
					crtIndex++;
					return true;
				}
				else if (checkOperator2(check)) // verific daca combinatia intre cele doua citite pana acum este valida ca operator separat.
				{
					reset();
					crtIndex++;
					return true;
				}
				else // altfel, printez doar operatorul curent si exclud caracterul citit acum.
				{
					reset();
					return true;
					crtIndex--;
				}
			}
			else // daca nu intalnesc operator, finalizez token-ul si exclud caracterul curent.
			{
				reset();
				return true;
				crtIndex--;
			}

		}
		else // Nu am gasit nicio tranzitie valida.
		{
			cout << "EROARE" << '\n';
			cout << "Nu a fost gasita nicio tranzitie valida" << '\n';
			exit(666);
		}

		crtIndex++; // Continui citirea.

	}

	// Daca la finalizarea fisierului, buffer-ul meu nu este gol,
	// atunci e posibil sa am exceptii.

	if (buffer != "")
	{
		if (getCurrentState().find("Parsing") == 0) // Daca asteptam inchidere unui comentariu, string etc., este eroare.
		{
			reset();
			return true;
			//error(buffer);
			//cout << "Fisierul s-a terminat asteptand o tranzitie din " << getCurrentState() << " catre o stare finala." << "\n---------------------------\n";
		}
		else // Finalizam ce a ramas in buffer.
		{
			reset();
			return true;
		}
	}

	reset();
	return false;
}

Token DFA::getNextToken(const string& nameOfFile, unsigned int& crtIndex)
{

	ifstream inFile;
	inFile.open(nameOfFile.c_str()); // Deschidem fisierul de input

	stringstream strStream;
	strStream << inFile.rdbuf(); // Citim fisierul cu ajutorul unui flux de tip string
	string inputFile = strStream.str(); // Stocam continutul in variabila 'inputFile'

	//unsigned int crtIndex = 0; // Indexul curent la care ne aflam in fisierul input
	char currentCh; // O variabila ce tine caracterul curent

	while (crtIndex <= inputFile.length()) // Cat timp citim din fisier...
	{

		currentCh = inputFile[crtIndex];

		if (isspace(currentCh) && getCurrentState() == "Start") {
			// Sar peste spatiile goale daca ma aflu in starea Start.
		}
		else if (getCurrentState() == "Start") // Daca ma situez in starea Start...
		{
			if (currentCh == '0')
			{
				if (inputFile[crtIndex + 1] == 'x' || inputFile[crtIndex + 1] == 'X')
				{
					changeState("Hexa");
					buffer += currentCh;
					buffer += inputFile[crtIndex + 1];
					crtIndex++;
				}
				else if (checkOcta(inputFile[crtIndex + 1]))
				{
					changeState("Octa");
					buffer += currentCh;
				}
				else
				{
					changeState("Cifre");
					buffer += currentCh;
				}
			}
			else if (checkCifra(currentCh)) // ...si intalnesc o cifra, merg in starea Cifre.
			{
				changeState("Cifre");
				buffer += currentCh;
			}
			else if (currentCh == '.' && checkCifra(inputFile[crtIndex + 1])) // ...si intalnesc un punct urmat de cifra, merg in starea Float.
			{
				changeState("Float");
				buffer += currentCh;
			}
			else if (checkLitera(currentCh) || currentCh == '_') // ...si intalnesc litere sau '_', merg in starea Identificator.
			{
				changeState("Identificator");
				buffer += currentCh;
			}
			else if (currentCh == '/') // ... si intalnesc caracterul '/', verific ce urmeaza dupa acesta.
			{
				if (inputFile[crtIndex + 1] == '*') // Daca urmeaza '*', se realizeaza tranzitie in starea de asteptare a comentariului multiplu.
				{
					changeState("ParsingComentariuMultiplu");
					buffer += currentCh;
					crtIndex++;
					buffer += inputFile[crtIndex + 1];
				}
				else if (inputFile[crtIndex + 1] == '/') // Daca urmeaza inca un caracter '/', atunci intru in starea de comentariu simplu.
				{
					changeState("ComentariuSimplu");
					buffer += currentCh;
					crtIndex++;
					buffer += inputFile[crtIndex + 1];
				}
				else // Altfel, este clar ca am intalnit operatorul '/'.
				{
					changeState("Operator");
					buffer += currentCh;
				}
			}
			else if (currentCh == '"') // Intru in starea de asteptare a literalului string cu ghilimele.
			{
				changeState("ParsingLiteralString1");
				buffer += currentCh;
			}
			else if (currentCh == '\'') // Intru in starea de asteptare a literalului string cu apostrof.
			{
				changeState("ParsingLiteralString2");
				buffer += currentCh;
			}
			else if (checkOperator1(currentCh)) // Daca caracterul curent este un operator valid, intru in starea operator.
			{
				changeState("Operator");
				buffer += currentCh;
			}
			else if (checkSeparator(currentCh)) // Daca caracterul curent este un separator valid, intru in starea separator.
			{
				changeState("Separator");
				buffer += currentCh;

				currentToken.setType(getCurrentState()); // Token-ul curent primeste ca tip starea curenta, iar ca valoare primeste buffer-ul curent.
				
				//printToken(currentToken); // Se printeaza token-ul curent.
				crtIndex++;

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset(); // Se reseteaza starea la Start.

				return currentToken;
			}
			else if (currentCh == ' ') // Daca intalnim spatiu, continuam.
			{
				continue;
			}

		}
		else if (getCurrentState() == "Unsigned")
		{
			if (currentCh == 'l' || currentCh == 'L')
			{
				buffer += currentCh;
				changeState("LiteralUnsignedLong");

				if (inputFile[crtIndex + 1] == 'l' || inputFile[crtIndex + 1] == 'L' || inputFile[crtIndex + 1] == 'u' || inputFile[crtIndex + 1] == 'U')
				{
					buffer += inputFile[crtIndex + 1];
					changeState("ERROR");
					currentToken.setType(getCurrentState());

					int index = addToUniques(buffer);

					currentToken.setValue(index);
					return currentToken;
				}
				else if (checkLitera(inputFile[crtIndex + 1]) || inputFile[crtIndex + 1] == '_' || checkCifra(inputFile[crtIndex + 1])) // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
				{
					buffer += inputFile[crtIndex + 1];
					changeState("ERROR");
					currentToken.setType(getCurrentState());

					int index = addToUniques(buffer);

					currentToken.setValue(index);
					return currentToken;
				}
				else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
				{
					currentToken.setType(getCurrentState());
					//printToken(currentToken);

					int index = addToUniques(buffer);

					currentToken.setValue(index);
					reset();
					crtIndex++;
					return currentToken;
				}
			}
			else if (checkLitera(currentCh) || currentCh == '_' || checkCifra(currentCh)) // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
			{
				buffer += currentCh;
				changeState("ERROR");
				currentToken.setType(getCurrentState());

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				return currentToken;
			}
			else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
			{
				currentToken.setType(getCurrentState());
				//printToken(currentToken);

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
			}
		}
		else if (getCurrentState() == "Long")
		{
			if (currentCh == 'u' || currentCh == 'U')
			{
				buffer += currentCh;
				changeState("LiteralUnsignedLong");

				if (inputFile[crtIndex + 1] == 'l' || inputFile[crtIndex + 1] == 'L' || inputFile[crtIndex + 1] == 'u' || inputFile[crtIndex + 1] == 'U')
				{
					buffer += inputFile[crtIndex + 1];
					crtIndex++;
					changeState("ERROR");
					currentToken.setType(getCurrentState());

					int index = addToUniques(buffer);

					currentToken.setValue(index);
					return currentToken;
				}
				else if (checkLitera(inputFile[crtIndex + 1]) || inputFile[crtIndex + 1] == '_' || checkCifra(inputFile[crtIndex + 1])) // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
				{
					buffer += inputFile[crtIndex + 1];
					crtIndex++;
					changeState("ERROR");
					currentToken.setType(getCurrentState());

					int index = addToUniques(buffer);

					currentToken.setValue(index);
					return currentToken;
				}
				else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
				{
					currentToken.setType(getCurrentState());
					//printToken(currentToken);

					int index = addToUniques(buffer);

					currentToken.setValue(index);
					reset();
					crtIndex++;
					return currentToken;
				}
			}
			else if (checkLitera(currentCh) || currentCh == '_' || checkCifra(currentCh)) // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
			{
				buffer += currentCh;
				changeState("ERROR");
				currentToken.setType(getCurrentState());

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				return currentToken;
			}
			else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
			{
				currentToken.setType(getCurrentState());
				//printToken(currentToken);

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
			}
		}
		else if (getCurrentState() == "Cifre") // Suntem in starea Cifre...
		{
			if (currentCh == '.') // ... si citim un punct, trecem in starea Float.
			{
				changeState("Float");
				buffer += currentCh;
			}
			else if (checkCifra(currentCh)) // ... si citim o cifra, ramanem in aceeasi stare;
			{
				buffer += currentCh;
			}
			else if (currentCh == 'e' || currentCh == 'E') // ... si citim un 'e', trecem in starea de Exponential
			{
				changeState("Exponential");
				buffer += currentCh;
			}
			else if (currentCh == 'u' || currentCh == 'U')
			{
				changeState("Unsigned");
				buffer += currentCh;
			}
			else if (currentCh == 'l' || currentCh == 'L')
			{
				changeState("Long");
				buffer += currentCh;
			}
			else if (currentCh == ' ') // ... si citim spatiu, atunci finalizam cu acest token.
			{
				currentToken.setType(getCurrentState());
				//printToken(currentToken);

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();

				return currentToken;
			}
			else if (checkLitera(currentCh) || currentCh == '_') // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
			{
				buffer += currentCh;
				changeState("ERROR");
				currentToken.setType(getCurrentState());

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				return currentToken;
			}
			else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
			{
				currentToken.setType(getCurrentState());
				//printToken(currentToken);

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
				crtIndex--;
			}
		}
		else if (getCurrentState() == "Hexa")
		{
			if (checkHexa(currentCh))
			{
				buffer += currentCh;
			}
			else if (currentCh == ' ') // ... si citim spatiu, atunci finalizam cu acest token.
			{
				currentToken.setType(getCurrentState());
				//printToken(currentToken);

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();

				return currentToken;
			}
			else if (currentCh == 'u' || currentCh == 'U')
			{
				changeState("Unsigned");
				buffer += currentCh;
			}
			else if (currentCh == 'l' || currentCh == 'L')
			{
				changeState("Long");
				buffer += currentCh;
			}
			else if (!checkHexa(currentCh) && (checkLitera(currentCh) || currentCh == '_')) // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
			{
				buffer += currentCh;
				changeState("ERROR");
				currentToken.setType(getCurrentState());

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				return currentToken;
			}
			else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
			{
				currentToken.setType(getCurrentState());
				//printToken(currentToken);

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
				crtIndex--;
			}
		}
		else if (getCurrentState() == "Octa")
		{
			if (checkOcta(currentCh))
			{
				buffer += currentCh;
			}
			else if (currentCh == ' ') // ... si citim spatiu, atunci finalizam cu acest token.
			{
				currentToken.setType(getCurrentState());
				//printToken(currentToken);

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();

				return currentToken;
			}
			else if (currentCh == 'u' || currentCh == 'U')
			{
				changeState("Unsigned");
				buffer += currentCh;
			}
			else if (currentCh == 'l' || currentCh == 'L')
			{
				changeState("Long");
				buffer += currentCh;
			}
			else if (!checkOcta(currentCh) && (checkLitera(currentCh) || currentCh == '_' || checkInvalidOcta(currentCh))) // TRANZITIE INVALIDA DIN CIFRE CU LITERA SAU '_'
			{
				buffer += currentCh;
				changeState("ERROR");
				currentToken.setType(getCurrentState());

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				return currentToken;
			}
			else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
			{
				currentToken.setType(getCurrentState());
				//printToken(currentToken);

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
				crtIndex--;
			}
		}
		else if (getCurrentState() == "Float") // Din starea Float...
		{
			if (checkCifra(currentCh)) // ... cu cifra ramanem in Float.
			{
				buffer += currentCh;
			}
			else if (currentCh == 'e' || currentCh == 'E') // ... cu 'e' mergem in Exponential.
			{
				changeState("Exponential");
				buffer += currentCh;
			}
			else if (currentCh == 'u' || currentCh == 'U')
			{
				changeState("Unsigned");
				buffer += currentCh;
			}
			else if (currentCh == 'l' || currentCh == 'L')
			{
				changeState("Long");
				buffer += currentCh;
			}
			else if (checkLitera(currentCh) || currentCh == '_') // TRANZITIE INVALIDA DIN FLOAT CU LITERA SAU '_'
			{
				buffer += currentCh;
				changeState("ERROR");
				currentToken.setType(getCurrentState());

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
			}
			else // Nu ne mai foloseste caracterul actual, nu mai avem tranzitii posibile, deci finalizam si excludem caracterul curent citit.
			{
				currentToken.setType(getCurrentState());
				//printToken(currentToken);

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
				crtIndex--;
			}
		}
		else if (getCurrentState() == "Exponential") // Din starea Exponential...
		{
			if (currentCh == '-' || currentCh == '+' || checkCifra(currentCh)) // ... cu '+' sau '-' sau orice cifra, ramanem in aceeasi stare.
			{
				buffer += currentCh;
				crtIndex++;
				while (checkCifra(inputFile[crtIndex])) // Cat timp intalnesc cifre, le adaug in buffer.
				{
					buffer += inputFile[crtIndex];
					crtIndex++;
				}
				if (inputFile[crtIndex] == 'u' || inputFile[crtIndex] == 'U')
				{
					changeState("Unsigned");
					buffer += inputFile[crtIndex];
				}
				else if (inputFile[crtIndex] == 'l' || inputFile[crtIndex] == 'L')
				{
					changeState("Long");
					buffer += inputFile[crtIndex];
				}
				else if (checkLitera(inputFile[crtIndex]) || inputFile[crtIndex] == '_')
				{
					// Daca dupa cifrele citite am intalnit o litera sau '_' ...
					// Verific caracterul curent, iar daca acesta ma duce spre un identificator, este invalidata tranzitia.

					buffer += inputFile[crtIndex];
					changeState("ERROR");
					currentToken.setType(getCurrentState());

					int index = addToUniques(buffer);

					currentToken.setValue(index);
					reset();
					return currentToken;
				}
				else // Altfel, printez numarul rezultat si trec mai departe, excluzand caracterul curent parsat deja in verificare.
				{
					currentToken.setType(getCurrentState());
					//printToken(currentToken);

					int index = addToUniques(buffer);

					currentToken.setValue(index);
					reset();
					return currentToken;
					crtIndex--;
				}
			}
			else if (checkLitera(currentCh) || currentCh == '_') // TRANZITIE INVALIDA DIN FLOAT CU LITERA SAU '_'
			{
				buffer += currentCh;
				changeState("ERROR");
				currentToken.setType(getCurrentState());

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
			}
		}
		else if (getCurrentState() == "ParsingLiteralString1") // Sunt in asteptarea inchiderii unui literal string.
		{
			if (currentCh != '"' && currentCh != '\n') // Cat timp citesc caracterele string-ului, le adaug in buffer.
			{
				if (currentCh == '\\')
				{
					if (inputFile[crtIndex + 1] == '\r' && inputFile[crtIndex + 2] == '\n') // Linux coding
					{
						buffer += currentCh;
						buffer += inputFile[crtIndex + 1];
						buffer += inputFile[crtIndex + 2];
						crtIndex++;
						crtIndex++;
					}
					else if ((inputFile[crtIndex + 1] == '\n') || (inputFile[crtIndex + 1] == '\r'))
					{
						buffer += currentCh;
						buffer += inputFile[crtIndex + 1];
						crtIndex++;
					}
					else if (inputFile[crtIndex + 1] == '\"')
					{
						buffer += currentCh;
						buffer += inputFile[crtIndex + 1];
						crtIndex++;
					}
					else {
						buffer += currentCh;
					}
				}
				else {
					buffer += currentCh;
				}
			}
			else if (currentCh == '\n')
			{
				buffer += currentCh;
				changeState("ERROR");
				currentToken.setType(getCurrentState());

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
			}
			else // Am intalnit sfarsitul literalului.
			{
				buffer += currentCh;
				changeState("LiteralString");

				currentToken.setType(getCurrentState());
				//printToken(currentToken);				
				crtIndex++;

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
			}
		}
		else if (getCurrentState() == "ParsingLiteralString2") // Sunt in asteptarea inchiderii unui literal string.
		{
			if (currentCh != '\'' && currentCh != '\n') // Cat timp citesc caracterele string-ului, le adaug in buffer.
			{
				if (currentCh == '\\')
				{
					if (inputFile[crtIndex + 1] == '\'')
					{
						buffer += currentCh;
						buffer += inputFile[crtIndex + 1];
						crtIndex++;
					}
					else {
						buffer += currentCh;
					}
				}
				else {
					buffer += currentCh;
				}
			}
			else if (currentCh == '\n')
			{
				buffer += currentCh;
				changeState("ERROR");
				currentToken.setType(getCurrentState());

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
			}
			else // Am intalnit sfarsitul literalului.
			{
				changeState("LiteralString");
				buffer += currentCh;

				currentToken.setType(getCurrentState());
				//printToken(currentToken);

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				crtIndex++;
				reset();
				return currentToken;
			}
		}
		else if (getCurrentState() == "ComentariuSimplu") // Sunt intr-un comentariu simplu.
		{
			if (currentCh == '\n') // Cand ajung la finalul liniei curente, finalizez token-ul.
			{
				currentToken.setType(getCurrentState());
				//currentToken.setValue(buffer);
				//printToken(currentToken);
				reset();

				//return currentToken;
			}
			else // Altfel adaug caracterele in buffer.
			{
				buffer += currentCh;
			}
		}
		else if (getCurrentState() == "ParsingComentariuMultiplu") // Parsez  un comentariu multiplu.
		{
			if (currentCh == '*' && inputFile[crtIndex + 1] == '/') // Daca ii intalnesc sfarsitul, adaug '*/' gasite si finalizez.
			{
				buffer += currentCh;
				buffer += inputFile[crtIndex + 1];
				crtIndex++;
				//crtIndex++;

				changeState("ComentariuMultiplu");

				currentToken.setType(getCurrentState());
				//printToken(currentToken);
				reset();


				//return currentToken;
			}
			else // Altfel continui sa adaug pana la incheierea comentariului, ramanand tot in starea de asteptare.
			{
				buffer += currentCh;
			}
		}
		else if (getCurrentState() == "Identificator") // Daca sunt in starea de identificator...
		{
			if (checkLitera(currentCh) || currentCh == '_' || checkCifra(currentCh)) // cu litera sau '_', raman in aceeasi stare.
			{
				buffer += currentCh;
			}
			else // Am intalnit un caracter ce nu face parte din identificator, deci verific tipul token-ului curent si finalizez, excluzand caracterul curent.
			{
				if (checkKeyword(buffer))
				{
					changeState("Keyword");
				}
				else if (buffer == "true" || buffer == "false")
				{
					changeState("Boolean");
				}
				else if (checkSpecialOperator(buffer))
				{
					changeState("Operator");
				}
				currentToken.setType(getCurrentState());
				//printToken(currentToken);

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
				crtIndex--;
			}
		}
		else if (getCurrentState() == "Operator") // Sunt in starea de operator...
		{

			if (checkOperator1(currentCh)) // daca am mai citit inca un operator...
			{

				string copybuffer1 = buffer;
				string check = copybuffer1 += currentCh;
				string checkspecial = copybuffer1 += inputFile[crtIndex + 1];
				if (checkspecial == "<<=" || checkspecial == ">>=")
				{
					currentToken.setType(getCurrentState());
					int index = addToUniques(checkspecial);
					currentToken.setValue(index);
					reset();
					crtIndex++;
					crtIndex++;
					return currentToken;
				}
				else if (checkOperator2(check)) // verific daca combinatia intre cele doua citite pana acum este valida ca operator separat.
				{
					currentToken.setType(getCurrentState());
					//printToken(currentToken);

					int index = addToUniques(check);

					currentToken.setValue(index);
					reset();
					crtIndex++;
					return currentToken;
				}
				else // altfel, printez doar operatorul curent si exclud caracterul citit acum.
				{
					currentToken.setType(getCurrentState());
					//printToken(currentToken);

					int index = addToUniques(buffer);

					currentToken.setValue(index);
					reset();
					return currentToken;
					crtIndex--;

				}
			}
			else // daca nu intalnesc operator, finalizez token-ul si exclud caracterul curent.
			{
				currentToken.setType(getCurrentState());
				//printToken(currentToken);

				int index = addToUniques(buffer);

				currentToken.setValue(index);
				reset();
				return currentToken;
				crtIndex--;
			}

		}
		else // Nu am gasit nicio tranzitie valida.
		{
			cout << "EROARE" << '\n';
			cout << "Nu a fost gasita nicio tranzitie valida" << '\n';
			exit(666);
		}

		crtIndex++; // Continui citirea.

	}

	// Daca la finalizarea fisierului, buffer-ul meu nu este gol,
	// atunci e posibil sa am exceptii.

	if (buffer != "")
	{
		if (getCurrentState().find("Parsing") == 0) // Daca asteptam inchidere unui comentariu, string etc., este eroare.
		{
			changeState("ERROR");
			currentToken.setType(getCurrentState());
			//currentToken.setValue(buffer);
			cout << "Fisierul s-a terminat asteptand o tranzitie catre o stare finala." << "\n---------------------------\n";

			return currentToken;
		}
		else // Finalizam ce a ramas in buffer.
		{
			currentToken.setType(getCurrentState());
			//printToken(currentToken);

			int index = addToUniques(buffer);

			currentToken.setValue(index);
			return currentToken;
		}
	}
	return currentToken; // va fi gol;
}

int main(int argc, char* argv[]) {

	string nameOfFile = "exemplu.txt"; // Este denumirea implicita a fisierului tinta.
	DFA dfa = DFA(); // Declaram un automat.
	unsigned int crtIndexDFA = 0, crtIndexSCAN = 0;
	Token crtToken = Token();

	if (argv[1] != NULL) // Daca a fost introdus numele fisierului din linia de comanda, il preluam de acolo.
	{
		cout << argv[1] << " este numele fisierului.\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
		nameOfFile = argv[1];
	}

	while (dfa.scanToken(nameOfFile, crtIndexSCAN))
	{
		crtToken = dfa.getNextToken(nameOfFile, crtIndexDFA);
		if (crtToken.getValue() != -1)
		{
			dfa.printToken(crtToken);
		}
	}

	cout << "\n\n\nTABELA DE STRINGURI UNICE\n___________________________________________________\n\n";

	for (unsigned int i = 0; i < dfa.uniqueStrings.size(); i++)
	{
		cout << dfa.uniqueStrings[i].first << " ~~~ " << dfa.uniqueStrings[i].second << " \n";
	}

	return 0;
}