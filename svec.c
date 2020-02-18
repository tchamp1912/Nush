/* This file is lecture notes from CS 3650, Fall 2018 */
/* Author: Nat Tuck */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "svec.h"

svec*
make_svec()
{
    // correctly allocate and initialize data structure
    svec* sv = malloc(sizeof(svec));
    sv->data = malloc(4 * sizeof(char*));
    sv->size = 0;
		sv->capacity = 4;

    return sv;
}

void
free_svec(svec* sv)
{
    //free all allocated data
		//free each element of array
		int ii = 0;
		for(; ii < sv->size; ii++)
			free(sv->data[ii]);
		//free array
		free(sv->data);
		//free struct
		free(sv);
}

char*
svec_get(svec* sv, int ii)
{
    assert(ii >= 0 && ii < sv->size);
    return sv->data[ii];
}

void
svec_put(svec* sv, int ii, char* item)
{
    assert(ii >= 0 && ii < sv->size);
    // insert item into slot ii
		sv->data[ii] = strdup(item);

}

void
svec_push_back(svec* sv, char* item)
{
    int ii = sv->size;
    sv->size++;
    // expand vector if backing array
    // is not big enough

		//double size of vector if size meets capacity
		if(sv->size >= sv->capacity){

			//allocate an array twice the size
    	sv->data = realloc(sv->data, 2 * sv->capacity * sizeof(char*));
			sv->capacity *= 2;
		
		}
    //place new value
		svec_put(sv, ii, item);
}

void
svec_swap(svec* sv, int ii, int jj)
{
    // Swap the items in slots ii and jj
		char* tmp = sv->data[ii];
		sv->data[ii] = sv->data[jj];
		sv->data[jj] = tmp;
}


void
svec_reverse(svec* sv){
		
		// Reverse all elements in vector	
		int ii = 0;
		
		//swap front and back half of vector
		//convert from size to index (size - 1)
		for(; ii < sv->size/2; ii++)
			svec_swap(sv, ii, (sv->size-1) - ii);

}
