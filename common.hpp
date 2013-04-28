static const unsigned int letter_count[26]
{
	13, 3, 3, 6, 18, 3, 4, 3, 12, 2, 2, 5, 3, 8, 11, 3, 2, 9, 6, 9, 6, 3, 3, 2, 3, 2
//   A  B  C  D   E  F  G  H   I  J  K  L  M  N   O  P  Q  R  S  T  U  V  W  X  Y  Z
};

static const unsigned short default_port {57198};
static const sf::Uint8 protocol_version {0};

// insert x into list l at a random position
template <class T>
void random_insert(std::list<T>& l, T x)
{
	auto it = l.begin();
	auto pos = std::rand() % (l.size() + 1);
	for (unsigned int i = 0; i != pos && it != l.end(); it++, i++);
	l.insert(it, x);
}
