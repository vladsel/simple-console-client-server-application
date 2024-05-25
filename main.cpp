#include <string>
#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

using std::endl;
using std::cout;
using std::string;
using boost::asio::ip::tcp;

string header(int contentLength) {
	string h =
		"HTTP/1.1 200 OK\n"
		"Content-Length: " + boost::lexical_cast<string>(contentLength) + "\n"
		"Content-Type: txt;\n"
		"Connection: close\n"
		"\n";
	return h;
}

string read_file(const string &file_nm) {
	std::ifstream file;
	file.open(file_nm.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		throw std::ifstream::failure("\nImpossible to open the file\n");
	string str_file;
	std::array <char, 2048> buf{};
	while (file.read(buf.data(), sizeof(buf)).gcount() > 0)
		str_file.append(buf.data(), file.gcount());
	return str_file;
}

void server(const string file_nm) {
	try {

		boost::asio::io_context io_context;
		tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 1337));

		while (1) {
			cout << "Accepting connection on port 1337" << endl;
			tcp::socket socket(io_context);
			acceptor.accept(socket);

			cout << "Client connected" << endl;
			string transer_data = read_file(file_nm);

			boost::system::error_code error;

			boost::asio::write(socket, boost::asio::buffer(header(transer_data.size())), error);
			boost::asio::write(socket, boost::asio::buffer(transer_data), error);

			//acceptor.close();
			//socket.close();
		}
	}
	catch (std::exception &ex) {
		std::cerr << "\n" << ex.what() << endl;
	}
}

void client() {
	try {
		boost::asio::io_context io_context;
		tcp::resolver resolver{ io_context };

		auto end_points = resolver.resolve("127.0.0.1", "1337");

		tcp::socket socket{ io_context };
		boost::asio::connect(socket, end_points);

		while (1) {
			std::array <char, 2048> buf{};
			boost::system::error_code error;

			size_t len = socket.read_some(boost::asio::buffer(buf), error);

			if (error == boost::asio::error::eof)
				break;
			else if (error)
				throw boost::system::system_error(error);

			for (size_t i = 0; i < len; i++)
				cout << static_cast<char>(buf[i]);

			//cout.write(buf.data(), len);

			//resolver.cancel();
			//socket.close();
		}
	}
	catch (std::exception &ex) {
		std::cerr << "\n" << ex.what() << endl;
	}
}

int main(int argc, char *argv[]) {

	if (argv[1] != nullptr && argv[2] != nullptr) {

		//input to *argv[]: [1]: "client" or "server", [2]: "file_path".txt 
		if (strcmp(argv[1], "client") == 0)
			client();
		else if (strcmp(argv[1], "server") == 0)
			server(string(argv[2]));
		else
			std::cerr << "\nno client and no server" << endl;

	}

	return 0;
}