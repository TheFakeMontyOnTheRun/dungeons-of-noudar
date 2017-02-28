//
// http_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <asio/ip/tcp.hpp>

using asio::ip::tcp;

std::string ADDRESS = "127.0.0.1";
std::string PORT = "8080";

std::string playerId = "#";
std::string gameId = "1";
std::string dataStateCache = "";
int posX = 0;
int posY = 0;

int request( std::string command, std::function< void(std::string)> onRequestComplete );

void connectToServer() {
  request( "/GetGameId", [&](std::string result){
      playerId = result;
    });     
}

void readServerStatus() {

  std::stringstream command;
  command << "/GetGameStatus?gameId=1&playerId=";
  command << playerId;

  request( command.str(), [&](std::string result){
      dataStateCache = result;
    });     
}

void sendStatus() {
  char input = 0;

  std::cin >> input;

  switch (input) {

  case 'a':
    posX--;
    break;
    
  case 's':
    posX++;
    break;
    
  case 'w':
    posY--;
    break;
    
  case 'z':
    posY++;
    break;

  default:
    return;
  }

  std::stringstream command;
  command << "/SendMove?gameId=1&playerId=";
  command << playerId;
  command << "&x=" << posX;
  command << "&y=" << posY;

  request( command.str(), [&](std::string result){
      dataStateCache = result;
    });     
}

void printStatus() {
  int pos = 0;

  if ( dataStateCache.length() < 20 * 20 ) {
    return;
  }

  for ( int y = 0; y < 20; ++y ) {
    for ( int x = 0; x < 20; ++x ) {
      auto element = dataStateCache[ pos++ ];
      std::cout << element;

      if ( playerId[ 0 ] == element ) {
	posX = x;
	posY = y;
      }

    }
    std::cout << std::endl;
  } 
}


int main(int argc, char* argv[])
{

    connectToServer();

  while ( true ) {
    readServerStatus();
    printStatus();
    sendStatus();
  }

  return 0;
}
 
int request( std::string command, std::function< void(std::string)> onRequestComplete ) {

  try
  {
    std::stringstream ss;

    ss << ADDRESS;
    ss << ":";
    ss << PORT;

    asio::ip::tcp::iostream s( ADDRESS, PORT);

    // The entire sequence of I/O operations must complete within 60 seconds.
    // If an expiry occurs, the socket is automatically closed and the stream
    // becomes bad.
    s.expires_after(std::chrono::seconds(60));

    // Establish a connection to the server.
    s.connect( ADDRESS, "8080");

    if (!s)
    {
      std::cout << "Unable to connect to " << ss.str() << "  : " << s.error().message() << "\n";
      return 1;
    }

    // Send the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    s << "GET /multiplayer-server" << command << " HTTP/1.0\r\n";
    s << "Host: " << ADDRESS << "\r\n";
    s << "Accept: */*\r\n";
    s << "Connection: close\r\n\r\n";

    // By default, the stream is tied with itself. This means that the stream
    // automatically flush the buffered output before attempting a read. It is
    // not necessary not explicitly flush the stream at this point.

    // Check that response is OK.
    std::string http_version;
    s >> http_version;
    unsigned int status_code;
    s >> status_code;
    std::string status_message;
    std::getline(s, status_message);
    if (!s || http_version.substr(0, 5) != "HTTP/")
    {
      std::cout << "Invalid response\n";
      return 1;
    }

    if (status_code != 200)
    {
      std::cout << "Response returned with status code " << status_code << "\n";
      return 1;
    }

    // Process the response headers, which are terminated by a blank line.
    std::string header;

    while (std::getline(s, header) && header != "\r");

    // Write the remaining data to output.
    std::stringstream resultData;
    resultData << s.rdbuf();

    onRequestComplete( resultData.str() );
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }

  return 0;
}
