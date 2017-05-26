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

struct ResourceAddress
{
	std::string port;
	std::string host;
	std::string link;
};
std::string inosmi_link(const std::string & month, const std::string & year)
{
	std::string link;
	return link;
}
std::string ukraina_link(const std::string & month, const std::string & year)
{
	std::string link;
	return link;
}
std::string tass_link(const std::string & month, const std::string & year)
{
	std::string link;
	return link;
}
std::string rt_link(const std::string & month, const std::string & year)
{
	std::string link;
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

	/*std::string port = "http";
	std::string host = "ukraina.ru";
	std::string link = "/news/20170502/1018610569.html";*/
	
	try
	{
		std::vector<ResourceAddress> addresses_vector =
			download_from("d:/in_files/in_host_names.txt");
		std::ofstream out("d:/test_out_2.txt");

		out << client(port, host, link) << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << "\n";
	}

	return 0;
}