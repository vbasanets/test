#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
void show(int x)
{
	std::cout << x << " | ";
}
struct ResourceAddress
{
	std::string port;
	std::string host;
	std::string link;
};
int main()
{
	std::vector<std::string> vect;
	std::for_each(
		vect.begin(), vect.end(),
		[](std::string & addr) {addr.link = link_parse(addr.host, argv[2], argv[3]); }
	);
	return 0;
}