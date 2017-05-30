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

using boost::asio::ip::tcp;

class LinksClass
{
	std::string _port;
	std::string _host;
	std::string _month;
	std::string _year;
	std::string _dateFrom;
	std::string _dateTo;
	
	size_t _limit;
	std::string _limitStr;

	std::string _TegDateFrom;
	std::string _TegDateTo;
	std::string _TegQuery;
	std::string _TegOffset;
	
	std::string _firstGet;
public:
	LinksClass(
		const std::string & port,
		const std::string & host,
		const std::string & month,
		const std::string & year,
		const std::string & dateFrom,
		const std::string & dateTo)
		: _port(port), _host(host), _month(month), _year(year),
		_limit(0), _dateFrom(dateFrom), _dateTo(dateTo) { }
	void limit(size_t limit) { _limit = limit; }
	void port(const std::string & port) { _port = port; }
	void createFirstGet(
		const std::string & dateFrom,
		const std::string & dateTo,
		const std::string & query,
		const std::string & offset)
	{

	}
	void createNextGets(
		std::set<std::string> & selection,
		const std::string & dateFrom,
		const std::string & dateTo,
		const std::string & query,
		const std::string & offset)
	{

	}
};

template<typename T> std::string toStr(T x)
{
	std::ostringstream oss;
	oss << x;
	return oss.str();
}
template<typename T> T toNumber(const std::string & x) {
	std::istringstream iss(x);
	T n;
	iss >> n;
	return n;
}
template<typename T> T pullNumber(const std::string & x) {
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
	return toNumber<T>(number);
}
size_t countDaysInMonth(const std::string & month)
{
	if (month == "January" || month == "january" || month == "Jan" || month == "jan" ||
		month == "March" || month == "march" || month == "Mar" || month == "mar" ||
		month == "May" || month == "may" ||
		month == "July" || month == "july" || month == "Jul" || month == "jul" ||
		month == "August" || month == "august" || month == "Aug" || month == "aug" ||
		month == "October" || month == "october" || month == "Oct" || month == "oct" ||
		month == "December" || month == "december" || month == "Dec" || month == "dec")
		return 31;
	else if (month == "April" || month == "april" || month == "Apr" || month == "apr" ||
		month == "Jun" || month == "Jun" ||
		month == "September" || month == "september" || month == "Sep" || month == "sep" ||
		month == "November" || month == "november" || month == "Nov" || month == "nov")
		return 30;
	else if (month == "February" || month == "february" || month == "Feb" || month == "feb")
		return 28;
	else throw std::exception("Invalid month!!!");
}
std::string monthToNumber(const std::string & month)
{
	if (month == "January" || month == "january" || month == "Jan" || month == "jan")
		return "01";
	else if (month == "February" || month == "february" || month == "Feb" || month == "feb")
		return "02";
	else if (month == "March" || month == "march" || month == "Mar" || month == "mar")
		return "03";
	else if (month == "April" || month == "april" || month == "Apr" || month == "apr")
		return "04";
	else if (month == "May" || month == "may")
		return "05";
	else if (month == "Jun" || month == "Jun")
		return "06";
	else if (month == "July" || month == "july" || month == "Jul" || month == "jul")
		return "07";
	else if (month == "August" || month == "august" || month == "Aug" || month == "aug")
		return "08";
	else if (month == "September" || month == "september" || month == "Sep" || month == "sep")
		return "09";
	else if (month == "October" || month == "october" || month == "Oct" || month == "oct")
		return "10";
	else if (month == "November" || month == "november" || month == "Nov" || month == "nov")
		return "11";
	else if (month == "December" || month == "december" || month == "Dec" || month == "dec")
		return "12";
	else throw std::exception("Invalid month!!!");
}
std::string client(const std::string & host, const std::string & link)
{
	boost::asio::io_service io_service;
	// Get a list of endpoints corresponding to the server name.
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(host, "http");
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
	/*/
	while (std::getline(response_stream, header) && header != "\r")
		all_responce << header << "\n";
	//all_responce << "-------------------------------------------------------------------------------------------------------\n";
	if (response.size() > 0)
		all_responce << &response;
		*/
	// Read until EOF, writing data to output as we go.

	while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
		all_responce << &response << "\n";

	//all_responce << "-------------------------------------------------------------------------------------------------------\n";

	if (error != boost::asio::error::eof)
		throw boost::system::system_error(error);
	return all_responce.str();
}
std::string client(const std::string & host, const std::string & link, const std::string & port)
{

}
std::string convertAndEncoding(const std::string& data, const std::string& from, const std::string& to)
{
	if (data.empty())
	{
		return std::string("No text data!!!");
	}
	iconv_t convert_hnd = iconv_open(to.c_str(), from.c_str());
	if (convert_hnd == (iconv_t)(-1))
	{
		throw std::logic_error("Unable to create convertion descriptor");
	}

	const char* in_ptr = const_cast<char*>(data.c_str());
	std::size_t in_size = data.size();
	std::vector<char> outbuf(6 * data.size());
	char* out_ptr = &outbuf[0];
	std::size_t reverse_size = outbuf.size();

	std::size_t result = iconv(convert_hnd, &in_ptr, &in_size, &out_ptr, &reverse_size);

	iconv_close(convert_hnd);
	if (result == (std::size_t)(-1))
	{
		throw std::logic_error("unable to convert");
	}

	return std::string(outbuf.data(), outbuf.size() - reverse_size);
}
void inaSingleString(std::string & row)
{
	std::string temp, str;
	std::istringstream iss(row);
	while (iss >> temp)
		str += (temp + ' ');
	row.clear();
	row = str;
}
void decodingInaSingleString(std::string & row)
{
	std::string temp, str;
	std::istringstream iss(row);
	std::ofstream out("d:/log.txt");
	while (iss >> temp)
	{
		std::string characters;
		try
		{
			temp = convertAndEncoding(temp, "UTF-8", "CP1251");
		}
		catch (std::exception & ex)
		{
			out << "Exception conversion: " << ex.what() << "\t" << temp << std::endl;
			std::for_each(temp.begin(), temp.end(), [&characters](char ch) {
				characters += ch;
				characters = boost::regex_replace(characters, boost::regex(R"#([\!-\}]*)#"),
					"$0", boost::regex_constants::format_no_copy); });
			temp = characters;
		}
		str += (temp + " ");
	}
	row = str;
}
size_t integerPath(size_t sum, size_t limit)
{
	if (sum % limit != 0)
		return sum / limit + 1;
	else
		return sum / limit;
}
void timeDelay(size_t sec)
{
	clock_t delay = sec * CLOCKS_PER_SEC;
	clock_t start = clock();
	while (clock() - start < delay);
}
void testOut(const std::string & str, const std::string & fileName)
{
	std::ofstream out(fileName.c_str());
	out << str << std::endl;
}
/*
****************************************************
argv[1] - name Host
argv[2] - Month
argv[3] - year

****************************************************
*/
int main(int argc, char * argv[])
{
	setlocale(0, "");
	if (argc != 4)
	{
		std::cout << "Usage: <mount> <year>\n";
		std::cout << "Example:\n";
		std::cout << " may 2017\n";
		return 1;
	}
	std::string host(argv[1]);
	std::string month(argv[2]);
	std::string year(argv[3]);
	std::string monthNumerical = monthToNumber(month);
	size_t daysInMonth = countDaysInMonth(month);
	std::set<std::string> selection;
	try
	{
		if (host == "inosmi.ru")
		{
			int limit = 100;
			std::string sufix =
				"/services/search/getmore/?search_area=all&date_from=" +
				year + "-" + monthNumerical +"-01" +
				"&date_to=" +
				year + "-" + monthNumerical + "-" + toStr(daysInMonth) +
				"&query%5B%5D=%D1%83%D0%BA%D1%80%D0%B0%D0%B8%D0%BD%D0%B0&limit=" +
				toStr(limit);
			std::string prefix = "&offset=";
			
			std::string raw = client(host, sufix + prefix + toStr(0));
			decodingInaSingleString(raw);
			
			boost::regex re(R"#(<div\sclass=\"search__results\">.*?<span>(.*?)</span>)#");
			raw = boost::regex_replace(raw, re, "$1", boost::regex_constants::format_no_copy);
			
			int sumAllPosts = pullNumber<int>(raw);
			int sumRequests = integerPath(sumAllPosts, limit);
			
			srand((unsigned int)time(0));
			for (int i = 0; i < sumRequests; ++i)
			{
				raw = client(host, sufix + prefix + toStr(i*limit));
				timeDelay(rand() / 5000);
			}

			testOut(raw, "d:/outFile.txt");
		}
		else if (host == "ukraina.ru")
		{
			
			
		}
		else if (host == "russian.rt.com")
		{
			
		}
		else
		{
			
		}
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << "\n";
	}

	return 0;
}