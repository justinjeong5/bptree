/**
 * 2020.7.2
 * DB_bptree 12164720 정경하
 */

#include <iostream>
#include <fstream>
#include <memory>

using namespace std;

class node {
protected:
	unique_ptr<char[]> memory_chunk;
	const unsigned block_size;

	unsigned _size;

	unsigned find_key_loc(int key) const {
		unsigned loc = 0;
		for (; loc < _size; loc++) {
			if (get_key(loc) >= key) break;
		}
		return loc;
	}

public:
	const unsigned cap;
	const unsigned bid;

	node(unique_ptr<char[]> mem_chunk, const unsigned blk_size, const unsigned bid)
		: memory_chunk{ move(mem_chunk) }, block_size{ blk_size }, cap{ block_size / 8 }, bid{ bid }, _size{ 0 } {}


	bool is_full() const {
		return _size == cap;
	}

	size_t size() const {
		return _size;
	}

	virtual int get_key(unsigned idx) const = 0;

	void update_to(fstream &stream) const {
		stream.seekp(12 + (bid - 1) * block_size, ios::beg);
		stream.write(memory_chunk.get(), block_size);
		stream.clear();
	}

};

class leaf_node : public node {
protected:
	unsigned _next_bid;

public:
	typedef pair<int, int> pair;

	leaf_node(unique_ptr<char[]> mem_chunk, const unsigned blk_size, const unsigned bid)
		: node{ move(mem_chunk), blk_size, bid },
		_next_bid{ *reinterpret_cast<unsigned *>(&memory_chunk[block_size - 4]) } {
		int *arr = reinterpret_cast<int *>(memory_chunk.get());

		for (; _size < cap; ++_size) {
			if (!(_size == 0 && arr[2] != 0) && arr[_size * 2] == 0) break;
		}
	}


	int get_key(const unsigned idx) const override {
		if (idx > _size) {
			throw out_of_range{ "exceeded the range of block." };
		}

		return reinterpret_cast<int *>(memory_chunk.get())[idx * 2];
	}

	const pair *operator[](size_t idx) const {
		if (idx > _size) {
			throw out_of_range{ "exceeded the range of block." };
		}

		return reinterpret_cast<const pair *>(memory_chunk.get()) + idx;
	}

	bool add(const int key, const int value) {
		if (_size == cap) return false;

		const unsigned loc = find_key_loc(key);
		if (get_key(loc) == key) return false;

		int *arr = reinterpret_cast<int *>(memory_chunk.get());
		if (loc != _size) {
			copy_backward(arr + (loc * 2), arr + (_size * 2), arr + (_size * 2) + 2);
		}
		arr[loc * 2] = key;
		arr[(loc * 2) + 1] = value;
		_size++;

		return true;
	}

	leaf_node split_n_add(const unsigned new_bid, const int key, const int value) {
		unsigned new_key_loc = find_key_loc(key);

		int *const src = reinterpret_cast<int *>(memory_chunk.get());
		int* merged = new int[_size * 2 + 2];
		copy(src, src + new_key_loc * 2, merged);
		copy(src + new_key_loc * 2, src + _size * 2, merged + new_key_loc * 2 + 2);
		fill(src, src + _size * 2, 0);
		merged[new_key_loc * 2] = key;
		merged[new_key_loc * 2 + 1] = value;

		char *const p = new char[block_size]();
		int *const dst = reinterpret_cast<int *>(p);

		_size++;
		const unsigned half = (_size / 2) * 2;
		const unsigned end = _size * 2;

		copy(merged, merged + half, src);
		copy(merged + half, merged + end, dst);

		dst[cap * 2] = _next_bid;
		src[cap * 2] = _next_bid = new_bid;
		_size = half / 2;

		return leaf_node{ unique_ptr<char[]>(p), block_size, new_bid };
	}

	int find(const int key) const {
		int *arr = reinterpret_cast<int *>(memory_chunk.get());

		const unsigned loc = find_key_loc(key);
		if (get_key(loc) == key) return arr[(loc * 2) + 1];
		else return 0;
	}

	unsigned next_bid() const {
		return _next_bid;
	}
};

class internal_node : public node {
public:
	internal_node(unique_ptr<char[]> mem_chunk, const unsigned blk_size, const unsigned bid)
		: node{ move(mem_chunk), blk_size, bid } {
		int *arr = reinterpret_cast<int *>(memory_chunk.get());

		for (; _size < cap; ++_size) {
			if (arr[_size * 2 + 1] == 0) break;
		}
	}

	unsigned get_next_bid(const unsigned idx) const {
		if (idx > _size + 1) {
			throw out_of_range{ "exceeded the range of block." };
		}

		return *reinterpret_cast<int *>(&memory_chunk[idx * 8]);
	}

	int get_key(const unsigned idx) const override {
		if (idx > _size) {
			throw out_of_range{ "exceeded the range of block." };
		}

		return *reinterpret_cast<int *>(&memory_chunk[idx * 8 + 4]);
	}

	bool add(const int key, const unsigned next_level_bid) {
		if (_size == cap) return false;

		const unsigned loc = find_key_loc(key);

		int *arr = reinterpret_cast<int *>(memory_chunk.get()) + 1;
		if (loc != _size) {
			copy_backward(arr + loc, arr + (_size * 2), arr + (_size * 2) + 2);
		}
		arr[loc * 2] = key;
		arr[(loc * 2) + 1] = next_level_bid;
		_size++;

		return true;
	}

	pair<internal_node, unsigned> split_n_add(const unsigned new_bid, const int key,
		const unsigned next_level_bid) {
		const unsigned new_key_loc = find_key_loc(key);

		int *const src = reinterpret_cast<int *>(memory_chunk.get());
		int* merged = new int[_size * 2 + 3];
		move(src, src + new_key_loc * 2 + 1, merged);
		move(src + new_key_loc * 2 + 1, src + _size * 2 + 1, merged + new_key_loc * 2 + 3);
		fill(src, src + _size * 2 + 1, 0);
		merged[new_key_loc * 2 + 1] = key;
		merged[new_key_loc * 2 + 2] = next_level_bid;

		char *const p = new char[block_size]();
		int *const dst = reinterpret_cast<int *>(p);

		const unsigned end = (_size * 2 + 3);
		const unsigned half = end / 2;
		unsigned middle;

		if (_size %2 == 1) {							 		
			copy(merged, merged + half + 1, src);
			copy(merged + half + 3, merged + end, dst);
			_size = half / 2;
			middle = merged[half + 1];
		}
		else {												
			copy(merged, merged + half, src);
			copy(merged + half + 1, merged + end, dst);
			_size = half / 2;
			middle = merged[half];
		}

		return make_pair(internal_node{ unique_ptr<char[]>(p), block_size, new_bid }, middle);
	}
};

class b_tree {
protected:
	static const unsigned NULL_NODE = 0;

	fstream stream;
	unsigned block_size{};
	unsigned root_bid{};
	unsigned depth{};
	unsigned block_cnt;

	void update_header() {
		stream.seekp(sizeof(block_size), ios::beg);
		stream.write(reinterpret_cast<const char *>(&root_bid), sizeof(root_bid));
		stream.write(reinterpret_cast<const char *>(&depth), sizeof(depth));
	}

	unique_ptr<char[]> load_block(const unsigned bid) {
		unique_ptr<char[]> ptr{ new char[block_size]() };

		stream.seekp(12 + (bid - 1) * block_size, ios::beg);
		stream.read(ptr.get(), block_size);

		return ptr;
	}

	internal_node load_internal_node(const unsigned bid) {
		return internal_node{ load_block(bid), block_size, bid };
	}

	leaf_node load_leaf_node(const unsigned bid) {
		return leaf_node{ load_block(bid), block_size, bid };
	}

	leaf_node search(const int key, const unsigned cur_bid, const unsigned cur_depth) {
		if (cur_depth == this->depth) {
			return load_leaf_node(cur_bid);
		}

		const internal_node &node = load_internal_node(cur_bid);

		for (int i = 0; i < (int)node.size(); ++i) {
			if (node.get_key(i) > key) {
				return search(key, node.get_next_bid(i), cur_depth + 1);
			}
		}

		return search(key, node.get_next_bid(node.size()), cur_depth + 1);
	}

	pair<unsigned, int> handle_insert(internal_node &node, const int key, const int value,
		const unsigned next_bid, const unsigned cur_depth) {
		const pair<unsigned, int> &to_add = insert(key, value, next_bid, cur_depth);

		if (to_add.first == NULL_NODE) return to_add;
		else if (!node.is_full()) {
			node.add(to_add.second, to_add.first);
			node.update_to(stream);
			return make_pair(0, 0);
		}

		else {
			const pair<internal_node, unsigned int> &pair =
				node.split_n_add(++block_cnt, to_add.second, to_add.first);
			const internal_node &new_node = pair.first;
			node.update_to(stream);
			new_node.update_to(stream);

			return make_pair(new_node.bid, pair.second);
		}
	}

	pair<unsigned, int> insert(const int key, const int value, const unsigned cur_bid, const unsigned cur_depth) {
		if (cur_depth == this->depth) {
			leaf_node searched_node = load_leaf_node(cur_bid);

			if (!searched_node.is_full()) {
				if (searched_node.add(key, value)) {
					searched_node.update_to(stream);
				}

				return make_pair(0u, 0);
			}
			else {
				leaf_node new_node = searched_node.split_n_add(++block_cnt, key, value);
				searched_node.update_to(stream);
				new_node.update_to(stream);

				return make_pair(new_node.bid, new_node.get_key(0));
			}
		}

		internal_node node = load_internal_node(cur_bid);

		for (unsigned i = 0; i < (int)node.size(); ++i) {
			if (node.get_key(i) > key) {
				return handle_insert(node, key, value, node.get_next_bid(i), cur_depth + 1);
			}
		}

		return handle_insert(node, key, value, node.get_next_bid(node.size()), cur_depth + 1);
	}


public:
	explicit b_tree(const char *const file_path) : stream{ file_path, ios::binary | ios::in | ios::out } {
		stream.read(reinterpret_cast<char *>(&block_size), sizeof(block_size));
		stream.read(reinterpret_cast<char *>(&root_bid), sizeof(root_bid));
		stream.read(reinterpret_cast<char *>(&depth), sizeof(depth));
		block_cnt = stream.seekp(0, ios::end).tellp();
		block_cnt = (block_cnt - 12) / block_size;
		stream.clear();
	}

	~b_tree() {
		stream.close();
	}

	static void init(const char *const file_path, unsigned new_block_size) {
		ofstream stream{ file_path, ios::out | ios::binary | ios::trunc };

		stream.write(reinterpret_cast<const char *>(&new_block_size), sizeof(new_block_size));

		unsigned tmp;
		tmp = 1;
		stream.write(reinterpret_cast<const char *>(&tmp), sizeof(tmp));
		stream.write(reinterpret_cast<const char *>(&tmp), sizeof(tmp));

		tmp = 2;
		stream.write(reinterpret_cast<const char *>(&tmp), sizeof(tmp));
		new_block_size = new_block_size * 2 - 4;
		while (new_block_size--) stream.put('\0');

		stream.close();
	}

	int search(const int key) {
		const leaf_node &leaf_node = search(key, root_bid, 0);

		return leaf_node.find(key);
	}

	void insert(const int key, const int value) {
		const pair<unsigned, int> to_add = insert(key, value, root_bid, 0);

		if (to_add.first != NULL_NODE) {
			char *const p = new char[block_size]();
			int *arr = (int *)p;

			arr[0] = root_bid;
			arr[1] = to_add.second;
			arr[2] = to_add.first;

			internal_node new_root{ unique_ptr<char[]>(p), block_size, ++block_cnt };
			new_root.update_to(stream);

			depth++;
			root_bid = new_root.bid;
			update_header();
		}
	}

	void print(ofstream &output) {
		output << "<0>" << endl;
		const internal_node &root = load_internal_node(root_bid);

		for (unsigned i = 0; i < (int)root.size(); ++i) {
			output << root.get_key(i) << ',';
		}

		output.clear();
		output.seekp(-1, ios::cur);
		output << endl;

		output << "<1>" << endl;

		for (unsigned i = 0; i < (int)root.size(); ++i) {
			if (depth == 1) {
				leaf_node node = load_leaf_node(root.get_next_bid(i));

				for (unsigned j = 0; j < (int)node.size(); ++j) {
					output << node.get_key(j) << ',';
				}
			}
			else {
				internal_node node = load_internal_node(root.get_next_bid(i));

				for (unsigned j = 0; j < (int)node.size(); ++j) {
					output << node.get_key(j) << ',';
				}
			}
		}

		if (depth == 1) {
			leaf_node node = load_leaf_node(root.get_next_bid(root.size()));

			for (unsigned j = 0; j < (int)node.size(); ++j) {
				output << node.get_key(j) << ',';
			}
		}
		else {
			internal_node node = load_internal_node(root.get_next_bid(root.size()));

			for (unsigned j = 0; j < (int)node.size(); ++j) {
				output << node.get_key(j) << ',';
			}
		}

		output.clear();
		output.seekp(-1, ios::cur);
		output << endl;
	}

	void ranged_search(const int key1, const int key2, ofstream &output) {
		unsigned next_bid = search(key1, root_bid, 0).bid;

		bool break_flag = false;
		do {
			const leaf_node &node = load_leaf_node(next_bid);

			for (int i = 0; i < (int)node.size(); ++i) {
				const int key = node.get_key(i);
				if (key1 <= key && key <= key2) {
					const leaf_node::pair *const pair = node[i];

					output << pair->first << ',' << pair->second << '\t';
				}
				else if (key > key2) {
					break_flag = true;
					break;
				}
			}

			next_bid = node.next_bid();
		} while (!break_flag && next_bid != NULL_NODE);

		output.clear();
		output.seekp(-1, ios::cur);
		output << endl;
	}
};

void print_usage(char *filename) {
    cerr << "Usage: " << filename << " (c|i|s|r|p) <arguments>\n" << endl;
    exit(1);
}

int main(int argc, char *argv[]) {
	
	if (argc < 2) print_usage(argv[0]);

    switch (argv[1][0]) {
        case 'c':
            if (argv[1][1] != '\0' || argc != 4) print_usage(argv[0]);

            b_tree::init(argv[2], strtol(argv[3], nullptr, 10));
            break;

        case 'i': {
            if (argv[1][1] != '\0' || argc != 4) print_usage(argv[0]);

            b_tree tree{argv[2]};
            int k, v;
            FILE *const fp = fopen(argv[3], "r");
            while (fscanf(fp, "%d,%d", &k, &v) != -1) {
                tree.insert(k, v);
            }
            fclose(fp);
            break;
        }

        case 's': {
            if (argv[1][1] != '\0' || argc != 5) print_usage(argv[0]);

            b_tree tree{argv[2]};
            ifstream input{argv[3]};
            ofstream output{argv[4]};
            while (! input.eof()) {
                int key;
                input >> key;
                if (input.fail()) break;

                output << key << ',' << tree.search(key) << endl;
            }
            break;
        }

        case 'p': {
            if (argv[1][1] != '\0' || argc != 4) print_usage(argv[0]);

            b_tree tree{argv[2]};
            ofstream output = ofstream{argv[3]};
            tree.print(output);
            break;
        }

        case 'r': {
            if (argv[1][1] != '\0' || argc != 5) print_usage(argv[0]);

            b_tree tree{argv[2]};
            int k1, k2;
            FILE *const input = fopen(argv[3], "r");
            ofstream output{argv[4]};
            while (fscanf(input, "%d,%d", &k1, &k2) != -1) {
                tree.ranged_search(k1, k2, output);
            }
            fclose(input);
            break;
        }
    }

    return 0;
}