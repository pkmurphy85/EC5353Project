#include <iostream>
#include "qssnake.hpp"
using namespace std;
//Bulk of snake game code from https://github.com/Regalis/QSSnake
int main(int argc, char** argv) {
	QApplication main_app(argc, argv);
	QSSnake qssnake;
	qssnake.show();
	return main_app.exec();
}
