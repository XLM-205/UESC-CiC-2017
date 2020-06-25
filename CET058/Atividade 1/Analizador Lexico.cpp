/*-------Activity 01: Lexical Analysis-------
*	String recognition based on Automata and Trie-Dictionary solutions
*
*	Moon Wiz Studios (c) - 02/11/2019
*	by: Ramon Darwich de Menezes
*
*	YOU MAY NOT use this file for commercial purposes without permission from this file creator OR Moon Wiz Studios
*	YOU MAY use it in any project of your own or edit this file, given the proper credits to Moon Wiz Studios
*   This notice MAY NOT be removed nor altered from any source distribution
*
*/

// !! REQUIRES C++11 TO COMPILE !!
// Libraries Repository: https://github.com/XLM-205/Libs

#include <stdio.h>
#include <stdlib.h>

#include "Parsers\Lexicon.h"			//Lexical stuff
#include "Control\CustomTypes-SA.h"		//Everything else
#include "Control\DataTypes.h"			//Data types (List, Automata, Trie, etc..)
#include "Control\FileStates.h"			//File Reading

//#define AUT_DEBUG	//If uncommented, when traversing trough an automata, the path taken will be printed out

//File Path for the input file
#define F_PATH		 "Expr2.rpp"
//Dictionary path that contains the Lexigram Defintion, used when using the Trie-Dictionary Solution
#define LEX_DEF_PATH "C:\\Users\\Ramon\\OneDrive\\GPX\\Custom Libraries\\Libraries\\Parsers\\Lexigram Definition.lxd"
//The token that defines if this line is just a comment
#define COMMENT_TOKEN '#'
//The format used when printing tokens. Should have AT LEAST two '%s'
#define TOK_FORMAT "< %-18s, %5s >\n"

#ifdef AUT_DEBUG
	#define EXC_FUNC d_Execute
#else
	#define EXC_FUNC Execute
#endif

// Prototypes -------------------------------------------------------------------------------------
enum TokenType
{
	TYPE_UNKN = -1,	//Token is unknown (!)
	TYPE_CONS,		//Token is a constant, either a string or a plain number
	TYPE_VAR,		//Token is a variable
	TYPE_OP,		//Token is a operator, either a math or a logic one
	TYPE_FUNC,		//Token is a function
};

//Token Definition
class Token
{
private:
	const TokenType m_type;	//Token type
	LexSymbol *m_lexicon;	//Lexycon of this token
	BaseString *m_def;		//Definition of this token, if 'm_lexicon' wans't supplied
	BaseString *m_refStr;	//String that generated this token

public:
	Token(TokenType t, LexSymbol *lex, const char *ref) : m_type(t), m_lexicon(lex), m_def(nullptr), m_refStr(new BaseString(ref)) { }
	Token(TokenType t, const char *def, const char *ref) : m_type(t), m_lexicon(nullptr), m_def(new BaseString(def)), m_refStr(new BaseString(ref)) { }
	~Token()
	{
		if (m_refStr)
		{
			delete m_refStr;
		}
		if (m_def)
		{
			delete m_def;
		}
	}

	void print(void)
	{
		if (m_lexicon)
		{
			printf(TOK_FORMAT, m_lexicon->Comment(), m_refStr->getString());
		}
		else
		{
			printf(TOK_FORMAT, m_def->getString(), m_refStr->getString());
		}
	}
};

bool isWordSeparator(char);							//Returns true if a the char supplied is a word seperator token
List<BaseString>* subStr(char*);					//Returns a list all word substrings from the supplied string, all lowercase
List<Token>* catchToken(char*, List<Automata>*);	//Catch and filter tokens (Automata Variant)
List<Token>* catchToken(char*, LexDictionary*);		//Catch and filter tokens (Trie-Dictionary Variant)

List<List<Automata>> g_Automatas;					//Holds all automatas created to safely delete all afterwards

bool isWordSeparator(char C)
{
	switch (C)
	{
	case ',':
	case ' ':
	case '?':
	case ':':
	case ';':
	case '\t':
	case '\n':
		return true;
	}
	return false;
}

void linkAutomataLogic(Automata *stNode)
{
	const int EXP = 8, LEN = 3;
	char oper[EXP][LEN] = { "<", ">", "=", "|", "&", "~", "!", "^" };
	char operNames[EXP][5] = { "LT", "GT", "LEQ", "LOR", "LAND", "BNEG", "LNEG", "XOR" };
	g_Automatas.Add(new List<Automata>);
	for (int i = 0; i < EXP; i++)
	{
		char temp[5] = { '\0' };
		g_Automatas.Last()->Add(new Automata(true, operNames[i]));
		stNode->AddTransition(g_Automatas.Last()->Last(), oper[i][0]);
	}
	//Linking '<' at index 0 to '=' at index 2, so we can make '<='
	g_Automatas.Last()->GetAt(0)->AddTransition(g_Automatas.Last()->GetAt(2), '=');
	//Linking '>' at index 1 to '=' at index 2, so we can make '>='
	g_Automatas.Last()->GetAt(1)->AddTransition(g_Automatas.Last()->GetAt(2), '=');	
	//Linking '!' at index 6 to '=', so we can make '!='
	g_Automatas.Last()->GetAt(6)->AddTransition(g_Automatas.Last()->GetAt(2), '=');
	//Linking '|' at index 3 to itself, so we can make '||'
	g_Automatas.Last()->GetAt(3)->AddTransition('|');
	//Linking '&' at index 4 to itself, so we can make '&&'
	g_Automatas.Last()->GetAt(4)->AddTransition('&');
	//Linking '=' at index 2 to itself, so we can make '=='
	g_Automatas.Last()->GetAt(2)->AddTransition('=');
}

void linkAutomataMath(Automata *stNode)
{
	const int EXP = 4, LEN = 3;
	char oper[EXP][LEN] = { "+", "-", "*", "/" };
	char operNames[EXP][5] = { "Sum", "Sub.", "Mul.", "Div." };
	g_Automatas.Add(new List<Automata>);
	for (int i = 0; i < EXP; i++)
	{
		char temp[5] = { '\0' };
		g_Automatas.Last()->Add(new Automata(true, operNames[i]));
		stNode->AddTransition(g_Automatas.Last()->Last(), oper[i][0]);
	}
	//Linking '+' at index 0 to itself, so we can make '++'
	g_Automatas.Last()->GetAt(0)->AddTransition('+');
	//Linking '-' at index 1 to itself, so we can make '--'
	g_Automatas.Last()->GetAt(1)->AddTransition('-');
	//Linking '*' at index 2 to itself, so we can make '**'
	g_Automatas.Last()->GetAt(2)->AddTransition('*');
}

void linkAutomataWords(Automata *stNode)
{
	g_Automatas.Add(new List<Automata>);
	for (int i = 0; i < 26; i++)			//One node for each letters, all also an 'end' node
	{
		char temp[5] = { '\0' };				//Just to name each node correctly
		sprintf(temp, "qW%d", i + 1);
		g_Automatas.Last()->Add(new Automata(true, temp));
	}
	//Now we link each node on itself, and also, on the startring node
	for (int i = 0; i < 26; i++)
	{
		Automata *temp = g_Automatas.Last()->GetAt(i);
		for (int j = 0; j < 26; j++)
		{
			temp->AddTransition(g_Automatas.Last()->GetAt(j), 'a' + j);
		}
		stNode->AddTransition(temp, 'a' + i);
	}
}

void linkAutomataNumbers(Automata *stNode)
{
	g_Automatas.Add(new List<Automata>);
	for (int i = 0; i < 10; i++)			//One node for each number, all also an 'end' node
	{
		char temp[5] = {'\0'};				//Just to name each node correctly
		sprintf(temp, "qN%d", i + 1);
		g_Automatas.Last()->Add(new Automata(true, temp));
	}
	//Now we link each node on itself, and also, on the startring node
	for (int i = 0; i < 10; i++)
	{
		Automata *temp = g_Automatas.Last()->GetAt(i);
		for (int j = 0; j < 10; j++)
		{
			temp->AddTransition(g_Automatas.Last()->GetAt(j), '0' + j);
		}
		stNode->AddTransition(temp, '0' + i);
	}
}

//Letters ALWAYS return lowercase. Should receive a single line without the pesky '\n'
List<BaseString>* subStr(char *in)
{
	List<BaseString> *out = new List<BaseString>();
	int stx = 0;											//Start index
	for (int i = 0; in[i]; i++)
	{
		if (in[i] == COMMENT_TOKEN)							//Comment found. Abort whole line
		{
			return out;
		}
		in[i] = CharOperations::getLowerCase(in[i]);
		if (isWordSeparator(in[i]))							//We will only add a word when we find when it ends and mark the end of it with '\0'
		{
			in[i] = '\0';
			while (isWordSeparator(in[++i]))
			{
				if (in[i] == COMMENT_TOKEN)					//Comment found. Abort whole line
				{
					if (out->Count())						//Wait, did we added something or this line was a pure comment?
					{
						out->Add(new BaseString(&in[stx]));	//But since we came here, we should have data to insert
					}
					return out;
				}
				in[i] = '\0';
			}
			in[i] = CharOperations::getLowerCase(in[i]);	//Making sure EVERYTHING is lowercase
			if (in[stx])									//Only add a word if the word does not start with '\0' (empty string)
			{
				out->Add(new BaseString(&in[stx]));
			}
			if (in[i] == '\0')								//Reached the end of the input prematurely
			{
				return out;
			}
			stx = i;
		}
	}
	out->Add(new BaseString(&in[stx]));						//Add the last one!
	return out;
}

//Automata Variant
List<Token>* catchToken(char *in, List<Automata> *aut)
{
	List<BaseString> *temp = subStr(in);
	List<Token> *out = new List<Token>();
	for (int i = 0; i < temp->Count(); i++)
	{
		bool notFoundFlag = true;
		BaseString *tStr = temp->GetAt(i);
		for (int j = 0; j < aut->Count(); j++)
		{
			Automata *autom = aut->GetAt(j);
			Automata *res = nullptr;
			if (res = autom->EXC_FUNC(tStr->getString()))
			{
				notFoundFlag = false;
				switch (autom->Name()[1])
				{
				case 'N':	//Catched by the number automata
					out->Add(new Token(TokenType::TYPE_CONS, "Plain Number", tStr->getString()));
					break;
				case 'W':	//Catched by the word (letter) automata
					out->Add(new Token(TokenType::TYPE_VAR, "Word", tStr->getString()));
					break;
				case 'M':	//Catched by the math operator automata
					//Now check if it's a inc. / dec.
					if (tStr->getString()[1] == '+')
					{
						out->Add(new Token(TokenType::TYPE_OP, "Inc.", tStr->getString()));
					}
					else if (tStr->getString()[1] == '+')
					{
						out->Add(new Token(TokenType::TYPE_OP, "Dec.", tStr->getString()));
					}
					else
					{
						out->Add(new Token(TokenType::TYPE_OP, res->Name(), tStr->getString()));
					}
					break;
				case 'L':	//Catched by the math operator automata
					//Now check for special operators ('<=', '>=', '!=')
					if (tStr->getString()[1] == '=')
					{
						switch (tStr->getString()[0])
						{
						case '<':
							out->Add(new Token(TokenType::TYPE_OP, "LET", tStr->getString()));
							break;
						case '>':
							out->Add(new Token(TokenType::TYPE_OP, "GET", tStr->getString()));
							break;
						case '!':
							out->Add(new Token(TokenType::TYPE_OP, "DIF", tStr->getString()));
							break;
						case '=':
							out->Add(new Token(TokenType::TYPE_OP, "EQ", tStr->getString()));
							break;
						default:
							out->Add(new Token(TokenType::TYPE_OP, res->Name(), tStr->getString()));
							break;
						}
					}
					else if (tStr->getString()[1] == '|')
					{
						out->Add(new Token(TokenType::TYPE_OP, "OR", tStr->getString()));
					}
					else if (tStr->getString()[1] == '&')
					{
						out->Add(new Token(TokenType::TYPE_OP, "AND", tStr->getString()));
					}
					else
					{
						out->Add(new Token(TokenType::TYPE_OP, res->Name(), tStr->getString()));
					}
					break;
				default:	//We shouldn't be here...
					out->Add(new Token(TokenType::TYPE_UNKN, "-UNKNOWN-", tStr->getString()));
					break;
				}
			}
		}
		if (notFoundFlag)
		{
			out->Add(new Token(TokenType::TYPE_UNKN, "-UNKNOWN-", tStr->getString()));
		}
	}
	delete temp;
	return out;
}

//Trie-Dictionary Variant
List<Token>* catchToken(char *in, LexDictionary *dict)
{
	List<BaseString> *temp = subStr(in);
	List<Token> *out = new List<Token>();
	for (int i = 0; i < temp->Count(); i++)
	{
		BaseString *tStr = temp->GetAt(i);
		if (CharOperations::isOperatorMath(tStr->getString()[0]))
		{
			out->Add(new Token(TokenType::TYPE_OP, dict->getSymbol(tStr->getString()), tStr->getString()));
		}
		else if (CharOperations::isNumber(tStr->getString()[0]))	//If the first char is a number, the rest of it is a number too
		{
			out->Add(new Token(TokenType::TYPE_CONS, dict->getSymbol("__RES_CONS_NUM__"), tStr->getString()));
		}
		else
		{
			char temp[14] = { '\0' };
			sprintf(temp, "Variable-%d", dict->getAmountSymbolsAdded());
			out->Add(new Token(TokenType::TYPE_VAR, dict->Add(tStr->getString(), temp), tStr->getString()));
		}
	}
	delete temp;
	return out;
}

void TrieSolution(void)
{
	printf("(RUN.) [INF] - Method: Trie-Dictionary\n");
	BaseString FilePath(256);
	LexDictionary Dictionary(LEX_DEF_PATH);
	List<List<Token>> Tokens;
	FileStates reader(F_PATH, "r");
	if (!Dictionary.buildSuccess())	//Failed to build default Dictionary! Abort everything
	{
		printf("(DIC.) [ERR] - Failed to build Default Lexigram Dictionary! Aborting operation\n");
		getchar();
		exit(-1);
	}
	if (!reader.fileSuccess())
	{
		printf("(FIL.) [ERR] - Failed to open input file at: '%s' ! Aborting operation\n", F_PATH);
		getchar();
		exit(-2);
	}
	printf("(DIC.) [INF] - Lexigram Dictionary Successfuly built\n");
	while (!reader.endOfFile())
	{
		char *temp = reader.readLine();
		if (temp)
		{
			Tokens.Add(catchToken(temp, &Dictionary));
			delete[] temp;
		}
	}
	for (int i = 0; i < Tokens.Count(); i++)
	{
		printf("LINE[%3d]:\n", i + 1);
		List<Token> *temp = Tokens.GetAt(i);
		for (int j = 0; j < temp->Count(); j++)
		{
			printf("\t");
			temp->GetAt(j)->print();
		}
	}
	Tokens.Clear();
	printf("(RUN.) [INF] - Execution Ended as Expected. Token List Cleared\n");
	getchar();
	exit(0);
}

void AutomataSolution(void)
{
	printf("(RUN.) [INF] - Method: Automata\n");
	Automata *rootN = new Automata(false, "qN");	//Automata for numbers
	Automata *rootW = new Automata(false, "qW");	//Automata for words
	Automata *rootM = new Automata(false, "qM");	//Automata for Math Expressions
	Automata *rootL = new Automata(false, "qL");	//Automata for Logic Expressions (Including Atribution '=')
	List<List<Token>> Tokens;
	List<Automata> AutStarters;						//Automata starts. Used to check if a string is accepted
	FileStates reader(F_PATH, "r");
	if (!reader.fileSuccess())
	{
		printf("(FIL.) [ERR] - Failed to open input file at: '%s' ! Aborting operation\n", F_PATH);
		getchar();
		exit(-2);
	}
	linkAutomataNumbers(rootN);
	linkAutomataWords(rootW);
	linkAutomataMath(rootM);
	linkAutomataLogic(rootL);
	AutStarters.Add(rootN);
	AutStarters.Add(rootW);
	AutStarters.Add(rootM);
	AutStarters.Add(rootL);
	printf("(AUT.) [INF] - %d Automatas built\n", AutStarters.Count());
	while (!reader.endOfFile())
	{
		char *temp = reader.readLine();
		if (temp)
		{
			Tokens.Add(catchToken(temp, &AutStarters));
			delete[] temp;
		}
	}
	for (int i = 0; i < Tokens.Count(); i++)
	{
		printf("LINE[%3d]:\n", i + 1);
		List<Token> *temp = Tokens.GetAt(i);
		for (int j = 0; j < temp->Count(); j++)
		{
			printf("\t");
			temp->GetAt(j)->print();
		}
	}
	Tokens.Clear();
	printf("(RUN.) [INF] - Execution Ended as Expected. Token List Cleared\n");

	//Clean up and exit
	AutStarters.Clear();
	g_Automatas.Clear();
}

int main()
{
	//TrieSolution();
	AutomataSolution();
	getchar();
	return 0;
}