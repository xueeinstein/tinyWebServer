#include <iostream>
#include <stdio.h>
#include <cstring>
#include <time.h>
using namespace std;
int main()
{
	char webname[100];
	strcpy(webname, "a.html");
	cout << webname << endl;
	*webname = '\0';
	cout << webname << " end"<< endl;
time_t timep;
time (&timep);
cout << ctime(&timep);
return 0;
}