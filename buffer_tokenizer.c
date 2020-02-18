#include <stdio.h>
#include <string.h>

#include "buffer_tokenizer.h"
#include "svec.h"

/*
 * Unique Operators Strings:
 * "<"
 * ">"
 * "|"
 * "||"
 * "&"
 * "&&"
 * ";"
 *
 * Operator Characters:
 * "<"
 * ">"
 * "|"
 * "&"
 * ";"
 *
 * Parsing Precedence:
 * " " " (Quotations)
 * "|" 
 * "&"
 * ">" "<"
 * "&&" "||"
 * ";"
 */

static const char OPERATORS[5] = {'>', '<', '|', ';', '&'};
static const char SUBTOKENS[4] = {'\"', '(', '\"', ')'};

/*
 * This function ensures that the input buffer 
 * is not padded with leading whitespaces.
 */
char*
trim_leading_whitespace(char* buffer)
{
	//iterate over buffer till no whitespaces
	//are encountered
	int ii = 0;
	while (buffer[ii] == ' ')
		ii++;

	return buffer += ii;

}

/*
 * This function will tokenize the buffer array by
 * whitespaces and operators. Then save the tokenized 
 * strings to the sting vector passed to it.
 */
int 
tokenize(svec* tokens, char* buffer)
{
	
	//initialize necessary variables
	int ii = 0; 
	int doub;
	char op;
	char* temp;


	if(buffer[0] == '\0' && buffer[0] == '\n') return 1;

	buffer = trim_leading_whitespace(buffer);

	// iterate over buffer
	while(buffer[ii] != '\0' && buffer[ii] != '\n'){
		
			// check whitespace or tab
			if(buffer[ii] == ' ' || buffer[ii] == '\t'){
				//more than one whitespace in a row
				if(ii == 0){
					// once part of buffer is tokenized ignore all proceeding characters
					buffer ++;
					// zero index
					ii = 0;
				
				}

				else{
					// copy string from last whitespace to current whitespace
					temp = strndup(buffer, ii);
					// add string to string vector
					svec_push_back(tokens, temp);

					// check next character
					ii++;
					// once part of buffer is tokenized ignore all proceeding characters
					buffer += ii;
					// zero index
					ii = 0;
				}
	
			}
	
			else if((op = parse_single_operator(buffer[ii]))){
				// if a single operator is found check next for double
				// use bool to increment index and size variables

				doub = check_double_operator(op, buffer[ii + 1]);
				
				//if double op found increment index and size
				ii += doub;

				if(ii > doub){
					// add preceeding string to svec		
					temp = strndup(buffer, ii - doub);
					// add string to string vector
					svec_push_back(tokens, temp);
				}

				// add operator to svec
				temp = strndup((buffer + ii - doub), 1 + doub);
				// add string to string vector
				svec_push_back(tokens, temp);

				// move to next elements in array
				ii++;
				buffer += ii;
				// zero index
				ii = 0;

			}

			// quotations and parantheses signify sub-tokens
			// quotations signify command line strings 
			// parantheses signify sub-shell operations
			else if (buffer[ii] == SUBTOKENS[0] || buffer[ii] == SUBTOKENS[1]){
					// iterate until closing quotation or parenthesis is found
					int num_open_signifiers, num_closed_signifiers;
					char closing_signifier, open_signifier;
					closing_signifier = (buffer[ii] == SUBTOKENS[0]) ? SUBTOKENS[2]:SUBTOKENS[3];
					open_signifier = buffer[ii];

					// for every open signifier you find within sub-string or cmd
					// you must find that many closing signifiers
					num_open_signifiers = 0;
					num_closed_signifiers = 0;
					while(!(buffer[ii-1] == closing_signifier && (num_open_signifiers-num_closed_signifiers) == 0)){
						if(buffer[ii] == open_signifier) num_open_signifiers++;
						else if(buffer[ii] == closing_signifier) num_closed_signifiers++;
						ii++;
					}
					// once it is found add elements into string vector tokens		
					temp = strndup(buffer, ii);
					// add string to string vector
					svec_push_back(tokens, temp);

					ii++;
					buffer += ii;
					ii = 0;
			}
	
			// if no delimiter is found proceed
			else ii++;

	}

	//checks for base case of no input
	if(ii > 0){
		// once end up buffer is reached place remaining
		// elements into string vector tokens	
		temp = strndup(buffer, ii);
		// add string to string vector
		svec_push_back(tokens, temp);

	}

	return 0;

}

/*
 * This function checks if a character is a
 * shell operator.
 */
char
parse_single_operator(char buffer_element)
{
		//iterator over operators
		int ii = 0;
		for(; ii < 6; ii++){
			//check if it is an operator
			if(buffer_element == OPERATORS[ii])
				return buffer_element;
		}
		//return null pointer if not an operator
		return 0;
}

/*
 * This functions compares the operator
 * and the following character to determine
 * if it is a double operator. Returns boolean
 * int if so.
 */
int
check_double_operator(char op, char buffer_element)
{			
		return (op == buffer_element && op != ';') ? 1 : 0;
}

