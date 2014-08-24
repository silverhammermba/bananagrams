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
