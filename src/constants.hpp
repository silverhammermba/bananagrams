#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <SFML/Graphics.hpp> // TODO get rid of this shit

// TODO get rid of these fucking globals
extern sf::View gui_view;

// graphics
static const unsigned int PPB {48};

static const sf::Vector2i X    {1, 0};
static const sf::Vector2i Y    {0, 1};
static const sf::Vector2i XY   {1, 1};
static const sf::Vector2i ZERO {0, 0};

// networking
static const unsigned short default_server_port {57198}; // client port is server port + 1
static const sf::Uint8 protocol_version {0};

// packet types
static const sf::Uint8 cl_connect    {0};
static const sf::Uint8 cl_disconnect {1};
static const sf::Uint8 cl_ready      {2};
static const sf::Uint8 cl_check      {3};
static const sf::Uint8 cl_dump       {4};
static const sf::Uint8 cl_peel       {5};
static const sf::Uint8 cl_ack        {6};

static const sf::Uint8 sv_disconnect {0};
static const sf::Uint8 sv_info       {1};
static const sf::Uint8 sv_check      {2};
static const sf::Uint8 sv_dump       {3};
static const sf::Uint8 sv_peel       {4};
static const sf::Uint8 sv_done       {5};

#endif
