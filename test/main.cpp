//
// sync_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <fstream>
#include <vector>
#include <boost/asio.hpp>

#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")

using boost::asio::ip::tcp;

std::string query = "%D1%83%D0%BA%D1%80%D0%B0%D0%B8%D0%BD%D0%B0";

struct ResourceAddress
{
	std::string port;
	std::string host;
	std::string link;
};
template<typename T> std::string to_str(T x)
{
	std::ostringstream oss;
	oss << x;
	return oss.str();
}
template<typename T> T to_number(std::string x)
{
	std::istringstream iss(x);
	T n;
	iss >> n;
	return n;
}
template<typename T> T pull_number(std::string x)
{
	std::string number;
	std::istringstream iss(x);
	char ch;
	iss.get(ch);
	while (!iss.eof())
	{
		if (isdigit(ch))
			number += ch;
		iss.get(ch);
	}

	return to_number<T>(number);
}
std::string numbers_to_date(size_t x, size_t y, size_t z, const std::string & separator)
{
	std::ostringstream oss;
	if (x < 10) oss << "0" << x << separator;
	else oss << x << separator;
	if (y < 10) oss << "0" << y << separator;
	else oss << y << separator;
	if (z < 10) oss << "0" << z;
	else oss << z;
	return oss.str();
}
int days_in_month(size_t month)
{
	switch (month)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		return 31;
	case 4:
	case 6:
	case 9:
	case 11:
		return 30;
	case 2:
		return 29;
	default:
		throw std::exception("Invalid month!!!");
	}
}
std::string inosmi_link(const std::string & month, const std::string & year)
{
	size_t limit = 0;
	size_t month_ = to_number<size_t>(month);
	size_t year_ = to_number<size_t>(year);
	size_t all_days = days_in_month(month_);
	std::string link =
		"/services/search/getmore/?search_area=all&date_from=" + numbers_to_date(year_, month_, 1, "-") +
		"&date_to=" + numbers_to_date(year_, month_, all_days, "-") +
		"&query%5B%5D=" + query + "&limit=" + to_str(limit) + "&offset=";
	return link;
}
std::string ukraina_link(const std::string & month, const std::string & year)
{
	size_t month_ = to_number<size_t>(month);
	size_t year_ = to_number<size_t>(year);
	std::string link = "/archive/" + numbers_to_date(year_, month_, 1, "") + "/calendar.html";
	return link;
}
std::string tass_link(const std::string & month, const std::string & year)
{

	std::string link;
	return link;
}
std::string rt_link(const std::string & month, const std::string & year)
{
	size_t limit = 10;
	size_t month_ = to_number<size_t>(month);
	size_t year_ = to_number<size_t>(year);
	size_t all_days = days_in_month(month_);

	std::string link = "/search?q=" + query + "&type=News&df="
		+ numbers_to_date(1, month_, year_, "-") + "&dt="
		+ numbers_to_date(all_days, month_, year_, "-") + "&page=";
	return link;
}
std::string client(const std::string & port,
	const std::string & host, const std::string & link)
{
	boost::asio::io_service io_service;
	// Get a list of endpoints corresponding to the server name.
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(host, port);
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;

	// Try each endpoint until we successfully establish a connection.
	tcp::socket socket(io_service);
	boost::system::error_code error = boost::asio::error::host_not_found;
	while (error && endpoint_iterator != end)
	{
		socket.close();
		socket.connect(*endpoint_iterator++, error);
	}
	if (error)
		throw boost::system::system_error(error);

	// Form the request. We specify the "Connection: close" header so that the
	// server will close the socket after transmitting the response. This will
	// allow us to treat all data up until the EOF as the content.
	boost::asio::streambuf request;
	std::ostream request_stream(&request);
	request_stream << "GET " << link << " HTTP/1.0\r\n";
	request_stream << "Host: " << host << "\r\n";
	request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n\r\n";

	// Send the request.
	boost::asio::write(socket, request);

	// Read the response status line.
	boost::asio::streambuf response;
	boost::asio::read_until(socket, response, "\r\n");

	// Check that response is OK.
	std::istream response_stream(&response);
	std::string http_version;
	response_stream >> http_version;
	unsigned int status_code;
	response_stream >> status_code;
	std::string status_message;
	std::getline(response_stream, status_message);
	if (!response_stream || http_version.substr(0, 5) != "HTTP/")
		throw "Invalid response";
	if (status_code != 200)
		throw "Response returned with status code " + status_code;

	// Read the response headers, which are terminated by a blank line.
	boost::asio::read_until(socket, response, "\r\n\r\n");

	// Process the response headers.
	std::string header;
	std::ostringstream all_responce;
	while (std::getline(response_stream, header) && header != "\r")
		all_responce << header << "\n";
	all_responce << "-------------------------------------------------------------------------------------------------------\n";
	if (response.size() > 0)
		all_responce << &response;
	
	// Read until EOF, writing data to output as we go.
	
	while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
		all_responce << &response << "\n";
	all_responce << "-------------------------------------------------------------------------------------------------------\n";
	
	if (error != boost::asio::error::eof)
		throw boost::system::system_error(error);
	return all_responce.str();
}
std::vector<ResourceAddress> download_from(const std::string & resource_name)
{
	std::ifstream fin(resource_name.c_str());
	if (!fin)
	{
		std::cerr << "Can't open file \' " << resource_name << " \'" << std::endl;
		exit(EXIT_FAILURE);
	}
	std::string some_string;
	std::vector<ResourceAddress> addresses_vector;
	while (getline(fin, some_string))
	{
		std::istringstream iss(some_string);
		ResourceAddress one_address;
		iss >> one_address.port;
		iss >> one_address.host;
		addresses_vector.push_back(one_address);
	}
	return addresses_vector;
}
std::string link_parse(const std::string & host,
	const std::string & month, const std::string & year)
{
	std::string link;
	if (host == "inosmi.ru")
		link = inosmi_link(month, year);
	else if (host == "ukraina.ru")
		link = ukraina_link(month, year);
	else if (host == "tass.ru")
		link = tass_link(month, year);
	else
		link = rt_link(month, year);
	return link;
}
void upload_to(const std::string & resource_name)
{

	std::cout << "Upload completed successfully!" << std::endl;
}
int main(int argc, char * argv[])
{
	// argv[1] - file name with host and port names
	// argv[2] - Month in numerical form
	// argv[3] - year

	if (argc != 4)
	{
		std::cout << "Usage: <filename.txt> <mount> <year>\n";
		std::cout << "Example:\n";
		std::cout << " test.exe filename.txt 5 2017\n";
		return 1;
	}
		
	try
	{
		std::vector<ResourceAddress> addresses_vector =
			download_from("d:/in_files/in_host_names.txt");
		
		std::for_each( addresses_vector.begin(), addresses_vector.end(),
			[argv](ResourceAddress & addr) { addr.link = link_parse(addr.host, argv[2], argv[3]); }
		);


	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << "\n";
	}

	return 0;
}