// $Id$

/***********************************************************************
Moses - factored phrase-based language decoder
Copyright (C) 2006 University of Edinburgh

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************/

#include <cassert>
#include <limits>
#include <iostream>
#include <fstream>
#include "dictionary.h"
#include "n_gram.h"
#include "lmtable.h"
#include "lmmacro.h"


#include "LanguageModelKen.h"
#include "TypeDef.h"
#include "Util.h"
#include "FactorCollection.h"
#include "Phrase.h"
#include "InputFileStream.h"
#include "StaticData.h"

using namespace std;

namespace Moses
{

LanguageModelKen::LanguageModelKen(bool registerScore, ScoreIndexManager &scoreIndexManager, int dub)
:LanguageModelSingleFactor(registerScore, scoreIndexManager)
{
}

LanguageModelKen::~LanguageModelKen()
{
	delete m_ngram;
}


bool LanguageModelKen::Load(const std::string &filePath, 
			     FactorType factorType, 
			     size_t nGramOrder)
{
  cerr << "In LanguageModelKen::Load: nGramOrder = " << nGramOrder << "\n";
	m_ngram = new lm::ngram::Model(filePath.c_str());
	return true;
}

	/* get score of n-gram. n-gram should not be bigger than m_nGramOrder
	 * Specific implementation can return State and len data to be used in hypothesis pruning
	 * \param contextFactor n-gram to be scored
	 * \param finalState state used by LM. Return arg
	 * \param len ???
	 */	
float LanguageModelKen::GetValue(const vector<const Word*> &contextFactor, State* finalState, unsigned int* len) const
{

	FactorType	factorType = GetFactorType();
	size_t count = contextFactor.size();
	assert(count <= GetNGramOrder());
	if (count == 0)
	{
		finalState = NULL;
		return 0;
	}
	
	// set up context
	const vector<int> ngramId(count);
	for (size_t i = 0 ; i < count - 1 ; i++)
	{
		const Factor *factor = contextFactor[i]->GetFactor(factorType);
		const string &word = factor->GetString();
		
		//ngramId[i] = StringToId(word); FOR_KEN
	}

	float prob;
	//prob = m_ngram.GetScore(ngramId); FOR_KEN
  
	return TransformLMScore(prob);
}


}

