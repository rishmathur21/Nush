#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "svec.h"


// tokenizes a single string that lives in the buf before the bufIndex
char*
tokenize_word(char* buf, int bufIndex, int tokenStart)
{
	char* token = malloc(sizeof(char) * ((bufIndex - tokenStart) + 1));
	// copy that much of the input string into the token
	memcpy(token, buf + tokenStart, (bufIndex - tokenStart));
	// add null terminator to the string
	token[bufIndex - tokenStart] = 0;
	return token;
}


// handles the case where you see a space, tab or new line in the buffer
// returns where to read next once you tokenize
int
space_case(svec* xs, char* buf, int bufIndex, int tokenStart)
{
	// if we see another space...don't tokenize it...just move on
	if (buf[tokenStart] == ' ' || buf[tokenStart] == '\t'
		       	|| buf[tokenStart] == '\n')
	{
		tokenStart = tokenStart + 1;
		return tokenStart;
	}

	char* token = tokenize_word(buf, bufIndex, tokenStart);

	svec_push_back(xs, token);
	free(token);
	// increment where we will start reading the next token	
	tokenStart = bufIndex + 1;
	return tokenStart;	
}

// handles the case where you see a single symbol ( |, <, >, ;, &) in the buffer
// tokenizes the word that preceded the symbol and the symbol itself
// returns where to read next once you tokenize this
int
single_symbol_case(svec* xs, char* buf, int bufIndex, int tokenStart)
{
	
	char* token = tokenize_word(buf, bufIndex, tokenStart);
	
	// tokenize the symbol token
	char* symbolToken = malloc(sizeof(char) * 2);
	memcpy(symbolToken, buf + bufIndex, 1);
	symbolToken[1] = 0;

	// if the previous thing was a space only push pipe, otherwise push token and pipe
	if ((isspace(buf[bufIndex - 1]) || buf[bufIndex - 1] == '\t'))
	{

		tokenStart = bufIndex + 1;
		svec_push_back(xs, symbolToken);
		free(token);
		free(symbolToken);
	}
	else
	{
		tokenStart = bufIndex + 1;
		svec_push_back(xs, token);
		svec_push_back(xs, symbolToken);
		free(token);
		free(symbolToken);
	}
	return tokenStart;	
	
}

// handles the case where you see a double symbol ( ||, && ) in the buffer
// tokenizes the word that preceded the double symbol and the double symbol itself
// returns where to read next once you tokenize this
int double_symbol_case(svec* xs, char* buf, int bufIndex, int tokenStart)
{
	char* token = tokenize_word(buf, bufIndex, tokenStart);
	char* symbolToken;

	// if the previous thing was a space only push ||, otherwise push token and ||
	if ((isspace(buf[bufIndex - 1])) || buf[bufIndex - 1] == '\t')
	{
		symbolToken = tokenize_word(buf, bufIndex + 2, tokenStart);
	}
	else
	{
		symbolToken = tokenize_word(buf, bufIndex + 2, bufIndex - tokenStart);
		svec_push_back(xs, token);
	}
	tokenStart = bufIndex + 2;
	svec_push_back(xs, symbolToken);
	free(token);
	free(symbolToken);
	return tokenStart;

}


// reads a string character by character and assesses what needs to become a token
svec*
tokenize(char* buf) 
{
   svec* xs = make_svec();
   int size = strlen(buf);

   // where you will start reading the current token
   int tokenStart = 0;

   // iterate through the characters in the string and decide what to tokenize
   for (int ii = 0; ii < size; ii++)
   {
	if (buf[ii] == ';')
	{
		tokenStart = single_symbol_case(xs, buf, ii, tokenStart);
	}
	else if (buf[ii] == '|')
	{
		if(buf[ii + 1] == '|')
		{
			tokenStart = double_symbol_case(xs, buf, ii, tokenStart);
			ii = ii + 1; 
		}
		else 
		{
			tokenStart = single_symbol_case(xs, buf, ii, tokenStart);	
		}
	}
	else if (buf[ii] == '&')
	{
		if (buf[ii + 1] == '&')
		{
			tokenStart = double_symbol_case(xs, buf, ii, tokenStart);
			ii = ii + 1;
		}
		else {
			tokenStart = single_symbol_case(xs, buf, ii, tokenStart);
		}
	}
	else if (buf[ii] == '>' || buf[ii] == '<')
	{
		tokenStart = single_symbol_case(xs, buf, ii, tokenStart);
	}
	else if (isspace(buf[ii]) || buf[ii] == '\t' ||  buf[ii] == '\n')
	{
		tokenStart = space_case(xs, buf, ii, tokenStart);
	}
   }
   
   return xs;
}


// prints the vector of tokens in reverse order
void
print_vector(svec* xs, int nn)
{
	for (int ii = nn - 1; ii >= 0; ii--)
	{
		printf("%s\n", xs->data[ii]);
	}
	printf("\n");
	printf("tokens$ "); 

}

/*
int
main(int _ac, char* _av[])
{ 
	char buf[100]; 

	printf("tokens$ "); 
	while(fgets(buf, sizeof buf, stdin) != NULL)
	{

		svec* xs = tokenize(buf);
		print_vector(xs, xs->size);
		free_svec(xs);
	}
}
*/
