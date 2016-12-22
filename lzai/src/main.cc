/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.cc
 * Copyright (C) 2016 chen <chen@chen-X450CC>
 * 
 * lzai is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * lzai is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <cstdlib>

#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include <sqlite3.h>

#include <lzal.h>
#include <lzsql.h>

using namespace std;


int main(void)
{
	//char s1[512],s2[1024];
	std::string s1,s2;
	printf("init ok\n");
	while(1){
		
		getline(cin,s1);
		//test_sql();
		lz_sql_insert(s1);
		//cin.get(s1);
		//s2= atoi( s1.c_str() );
		s2=s1;
		//int num1 = std::atoi(s1.c_str());
		//printf("%s\n",s1);
		//cout<<s1;
		printf("result:");
		cout<<s2;
		printf("\ncin:");
		//printf("result:\n",s2);
	}
	return 0;
}

