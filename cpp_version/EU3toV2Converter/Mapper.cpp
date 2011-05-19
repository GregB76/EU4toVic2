// ****************************************************************************
// *																								  *
// *			  EU3 to Victoria 2 conversion project							*
// *																								  *
// ****************************************************************************


#include "Mapper.h"
#include "Log.h"
#include <algorithm>


provinceMapping initProvinceMap(Object* obj)
{
	provinceMapping mapping;
	provinceMapping::iterator mapIter;
	//set<string> blanks;

	vector<Object*> leaves = obj->getLeaves();

	if (leaves.size() < 1)
	{
		log ("Error: No province mapping definitions loaded.\n");
		printf("Error: No province mapping definitions loaded.\n");
		return mapping;
	}

	vector<Object*> data = leaves[0]->getLeaves();

	for (unsigned int i = 0; i < data.size(); i++)
	{
		vector<int> EU3nums;
		vector<int> V2nums;

		vector<Object*> euMaps = data[i]->getLeaves();

		for (unsigned int j = 0; j < euMaps.size(); j++)
		{
			if (euMaps[j]->getKey().compare("eu3") == 0)
			{
				EU3nums.push_back(atoi(euMaps[j]->getLeaf().c_str()));
			}
			else if (euMaps[j]->getKey().compare("vic") == 0)
			{
				V2nums.push_back(atoi(euMaps[j]->getLeaf().c_str()));
			}
			else
			{
				log("Warning: unknown data while mapping provinces.\n");
			}
		}

		for (unsigned int k = 0; k < V2nums.size(); k++)
		{
			pair< int, vector<int> > insertMe;
			insertMe.first = V2nums[k];
			insertMe.second = EU3nums;
			mapping.insert(insertMe);
		}
	}

	return mapping;
}


int initCountryMap(countryMapping& mapping, vector<string> EU3Tags, vector<string> V2Tags, Object* rulesObj)
{
	mapping.clear();
	countryMapping::iterator mapIter;

	// get rules
	vector<Object*>		leaves = rulesObj->getLeaves();
	if (leaves.size() < 1)
	{
		log ("Error: No country mapping definitions loaded.\n");
		printf("Error: No country mapping definitions loaded.\n");
		return -1;
	}
	vector<Object*> rules = leaves[0]->getLeaves();

	for (unsigned int i = 0; i < rules.size(); ++i)
	{
		vector<Object*> rule = rules[i]->getLeaves();
		string			rEU3Tag;
		vector<string>	rV2Tags;
		for (unsigned int j = 0; j < rule.size(); j++)
		{
			if (rule[j]->getKey().compare("eu3") == 0)
			{		 
				rEU3Tag = rule[j]->getLeaf();
			}
			else if (rule[j]->getKey().compare("vic") == 0)
			{
				rV2Tags.push_back(rule[j]->getLeaf());
			}
			else
			{
				log("Warning: unknown data while mapping countries.\n");
			}
		}

		//find EU3 tag from the rule
		vector<string>::iterator EU3TagPos = EU3Tags.end();
		for (vector<string>::iterator j = EU3Tags.begin(); j != EU3Tags.end(); j++)
		{
			if (*j == rEU3Tag)
			{
				EU3TagPos = j;
				break;
			}
		}
		if (EU3TagPos == EU3Tags.end())
		{
			continue;
		}

		//find V2 tag from the rule
		vector<string>::iterator V2TagPos = V2Tags.end();
		for (unsigned int j = 0; j < rV2Tags.size(); j++)
		{
			for (vector<string>::iterator k = V2Tags.begin(); k != V2Tags.end(); k++)
			{
				if (*k == rV2Tags[j])
				{
					V2TagPos = k;
					break;
				}
			}
		}
		if (V2TagPos == V2Tags.end())
		{
			continue;
		}

		//add the mapping
		mapping.insert(make_pair<string, string>(*EU3TagPos, *V2TagPos));
		log("Added map %s -> %s\n", EU3TagPos->c_str(), V2TagPos->c_str());

		//remove tags from the lists
		EU3Tags.erase(EU3TagPos);
		V2Tags.erase(V2TagPos);
	}

	while ( (EU3Tags.size() > 0) && (V2Tags.size() > 0) )
	{
		vector<string>::iterator EU3TagPos = EU3Tags.begin();
		if ( (*EU3TagPos == "REB") || (*EU3TagPos == "PIR") || (*EU3TagPos == "NAT") )
		{
			mapping.insert(make_pair<string, string>(*EU3TagPos, "REB"));
			EU3Tags.erase(EU3TagPos);
		}
		else
		{
			vector<string>::iterator V2TagPos = V2Tags.begin();
			mapping.insert(make_pair<string, string>(*EU3TagPos, *V2TagPos));

			//remove tags from the lists
			EU3Tags.erase(EU3TagPos);
			V2Tags.erase(V2TagPos);
		}
	}

	return EU3Tags.size();
}

void removeEmptyNations(EU3World& world)
{
	vector<EU3Country> countries = world.getCountries();

	for (unsigned int i = 0; i < countries.size(); i++)
	{
		vector<EU3Province*> provinces	= countries[i].getProvinces();
		vector<EU3Province*> cores			= countries[i].getCores();
		if ( (provinces.size()) == 0 && (cores.size() == 0) )
		{
			world.removeCountry(countries[i].getTag());
		}
	}
}


bool compareLandlessNationsAges(EU3Country A, EU3Country B)
{
	vector<EU3Province*> ACores = A.getCores();
	string ATag = A.getTag();
	date ADate;
	ADate.year	= 0;
	ADate.month	= 0;
	ADate.day	= 0;
	for (unsigned int i = 0; i < ACores.size(); i++)
	{
		date newADate	= ACores[i]->getLastPossessedDate(ATag);
		if (newADate.year > ADate.year)
		{
			ADate.year		= newADate.year;
			ADate.month		= newADate.month;
			ADate.day		= newADate.day;
		}
		else if ( (newADate.year == ADate.year) && (newADate.month > ADate.month))
		{
			ADate.year		= newADate.year;
			ADate.month		= newADate.month;
			ADate.day		= newADate.day;
		}
		else if ( (newADate.year == ADate.year) && (newADate.month == ADate.month) && (newADate.day > ADate.day))
		{
			ADate.year		= newADate.year;
			ADate.month		= newADate.month;
			ADate.day		= newADate.day;
		}
	}

	vector<EU3Province*> BCores = B.getCores();
	string BTag = B.getTag();
	date BDate;
	BDate.year	= 0;
	BDate.month	= 0;
	BDate.day	= 0;
	for (unsigned int i = 0; i < BCores.size(); i++)
	{
		date newBDate	= BCores[i]->getLastPossessedDate(BTag);
		if (newBDate.year > BDate.year)
		{
			BDate.year		= newBDate.year;
			BDate.month		= newBDate.month;
			BDate.day		= newBDate.day;
		}
		else if ( (newBDate.year == BDate.year) && (newBDate.month > BDate.month))
		{
			BDate.year		= newBDate.year;
			BDate.month		= newBDate.month;
			BDate.day		= newBDate.day;
		}
		else if ( (newBDate.year == BDate.year) && (newBDate.month == BDate.month) && (newBDate.day > BDate.day))
		{
			BDate.year		= newBDate.year;
			BDate.month		= newBDate.month;
			BDate.day		= newBDate.day;
		}
	}

	if (ADate.year < BDate.year)
	{
		return true;
	}
	else if ( (ADate.year == BDate.year) && (ADate.month < BDate.month))
	{
		return true;
	}
	else if ( (ADate.year == BDate.year) && (ADate.month == BDate.month) && (ADate.day < BDate.day))
	{
		return true;
	}
	else
	{
		return false;
	}
}


void removeOlderLandlessNations(EU3World& world, int& excess)
{
	vector<EU3Country> countries = world.getCountries();

	vector<EU3Country> countries2;
	for (unsigned int i = 0; i < countries.size(); i++)
	{
		vector<EU3Province*> provinces = countries[i].getProvinces();
		if (provinces.size() == 0)
		{
			countries2.push_back(countries[i]);
		}
	}

	sort(countries2.begin(), countries2.end(), compareLandlessNationsAges);

	while ( (excess > 0) && (countries2.size() > 0) )
	{
		world.removeCountry(countries2.back().getTag());
		countries2.pop_back();
		excess--;
	}
}


void removeLandlessNations(EU3World& world)
{
	vector<EU3Country> countries = world.getCountries();

	for (unsigned int i = 0; i < countries.size(); i++)
	{
		vector<EU3Province*> provinces = countries[i].getProvinces();
		if (provinces.size() == 0)
		{
			world.removeCountry(countries[i].getTag());
		}
	}
}


vector<string> getEU3Tags(EU3World srcWorld)
{
	vector<EU3Country>	EU3Countries = srcWorld.getCountries();
	vector<string>			EU3Tags;

	for (unsigned int i = 0; i < EU3Countries.size(); i++)
	{
		EU3Tags.push_back(EU3Countries[i].getTag());
	}

	return EU3Tags;
}


stateMapping initStateMap(Object* obj)
{
	stateMapping stateMap;
	vector<Object*> leaves = obj->getLeaves();

	for (unsigned int i = 0; i < leaves.size(); i++)
	{
		vector<string> provinces = leaves[i]->getTokens();
		vector<int>		neighbors;

		for (unsigned int j = 0; j < provinces.size(); j++)
		{
			neighbors.push_back(atoi(provinces[j].c_str()));
		}
		for (unsigned int j = 0; j < neighbors.size(); j++)
		{
			stateMap.insert(make_pair<int, vector<int> >(neighbors[j], neighbors));
		}
	}

	return stateMap;
}


cultureMapping initCultureMap(Object* obj)
{
	cultureMapping cultureMap;
	vector<Object*> links = obj->getLeaves();

	for (unsigned int i = 0; i < links.size(); i++)
	{
		vector<Object*>	cultures	= links[i]->getLeaves();
		string				dstCulture;
		vector<string>		srcCultures;

		for (unsigned int j = 0; j < cultures.size(); j++)
		{
			if (cultures[j]->getKey() == "vic")
			{
				dstCulture = cultures[j]->getLeaf();
			}
			if (cultures[j]->getKey() == "eu3")
			{
				srcCultures.push_back(cultures[j]->getLeaf());
			}
		}

		for (unsigned int j = 0; j < srcCultures.size(); j++)
		{
			cultureMap.insert(pair<string,string>(srcCultures[j], dstCulture));
		}
	}

	return cultureMap;
}


religionMapping initReligionMap(Object* obj)
{
	religionMapping religionMap;
	vector<Object*> links = obj->getLeaves();

	for (unsigned int i = 0; i < links.size(); i++)
	{
		vector<Object*>	religions	= links[i]->getLeaves();
		string				dstReligion;
		vector<string>		srcReligion;

		for (unsigned int j = 0; j < religions.size(); j++)
		{
			if (religions[j]->getKey() == "vic")
			{
				dstReligion = religions[j]->getLeaf();
			}
			if (religions[j]->getKey() == "eu3")
			{
				srcReligion.push_back(religions[j]->getLeaf());
			}
		}

		for (unsigned int j = 0; j < srcReligion.size(); j++)
		{
			religionMap.insert(pair<string,string>(srcReligion[j], dstReligion));
		}
	}

	return religionMap;
}