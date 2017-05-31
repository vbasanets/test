#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <ctime>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/regex.hpp>
#include <iconv.h>
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#pragma comment(lib, "libiconv.lib")

int main()
{
	
	srand((unsigned int)time(0));
	for (int i = 1; i != 100; i++)
		std::cout << rand()/5000 << " ";
	std::cout << std::endl;
	std::cout << CLOCKS_PER_SEC << std::endl;
	
	return 0;
}