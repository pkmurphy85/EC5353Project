#include "qssnake.hpp"
#include <QMenu>
#include <QStatusBar>
#include <QMenuBar>
#include <QIcon>
#include <QToolBar>
#include <QString>
#include <QPainter>
#include <QLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <iostream>
#include <ctime>
#include <QDebug>
#include <QMutex>
#include <QApplication>
#include <QWidget>

#include <linux/kernel.h> /* printk() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */

#include <stdio.h>
//#include <inttypes.h>  // uint8_t, etc
#include <stdint.h>

#include <string.h>
#include <QThread>



#include <fcntl.h>

#include <sys/ioctl.h>
#include "i2c-dev.h"

int ADS_address = 0x49;	// Address of our device on the I2C bus
char dirPart = 'Q';
QMutex mutex;
double usec = 10000;

using namespace std;

class Thread : public QThread
{
	public:
	    void run()
			{
				int I2CFile;
				I2CFile = ::open("/dev/i2c-0", O_RDWR);
				if (ioctl(I2CFile, I2C_SLAVE_FORCE, ADS_address) < 0) {
				   printf("Failed to acquire bus access and/or talk to slave.\n");
					/* ERROR HANDLING; you can check errno to see what went wrong */
					exit(1);
				}	

				while(1){
				usleep(usec);
				//mutex.lock();
				
				
				//char dirPart = 'Q';
				uint8_t writeBuf[3];		// Buffer to store the 3 bytes that we write to the I2C device
				uint8_t readBuf[2];		// 2 byte buffer to store the data read from the I2C device
				int maxIter = 0;

				int16_t val;				// Stores the 16 bit value of our ADC conversion
				int16_t val2;		


				
	
				
				 
			   	// printf("I2C open... \n");

			 
					// These three bytes are written to the ADS1115 to set the config register and start a conversion 
				  writeBuf[0] = 1;			// This sets the pointer register so that the following two bytes write to the config register
				  writeBuf[1] = 0xC3;   	// This sets the 8 MSBs of the config register (bits 15-8) to 11000011
				  writeBuf[2] = 0x03;  		// This sets the 8 LSBs of the config register (bits 7-0) to 00000011
				  //std::cout <<  (unsigned)writeBuf[0] << " " << (unsigned) writeBuf[1] << " " << (unsigned)writeBuf[2] << std::endl;
	
				  // Initialize the buffer used to read data from the ADS1115 to 0
				  readBuf[0]= 0;		
				  readBuf[1]= 0;
				  //printf("Read buf FIRST: %u \n",readBuf);
					  
				  // Write writeBuf to the ADS1115, the 3 specifies the number of bytes we are writing,
					//printf("Write Statement...\n");
				  // this begins a single conversion
				  ::write(I2CFile, writeBuf, 3);	

				  // Wait for the conversion to complete, this requires bit 15 to change from 0->1
				 while ((readBuf[0] & 0x80) == 0 && maxIter < 10)	// readBuf[0] contains 8 MSBs of config register, AND with 10000000 to select bit 15
				 {
					  //printf("Read Statement...\n");
					  ::read(I2CFile, readBuf, 2);	// Read the config register into readBuf
					  //printf("Read buf1: %u \n",(unsigned) readBuf[0]);
					  //printf("Read buf2: %d \n",readBuf[1]);
					  usleep(10000);
					  maxIter++;
					  //printf("Iteration: %d \n", maxIter);
					  	
				}
				maxIter =0;
		
				  writeBuf[0] = 0;					// Set pointer register to 0 to read from the conversion register
					//printf("Write Statement...\n");
				  ::write(I2CFile, writeBuf, 1);
				  //printf("Read Statement...\n");
				  ::read(I2CFile, readBuf, 2);		// Read the contents of the conversion register into readBuf

				  val = readBuf[0] << 8 | readBuf[1];	// Combine the two bytes of readBuf into a single 16 bit result 

				  printf("A0 ---- Voltage Reading %f (V) \n", (float)val*4.096/32767.0);	
	
			 
			   	// printf("I2C open... \n");

			 
					// These three bytes are written to the ADS1115 to set the config register and start a conversion 
				  writeBuf[0] = 1;			// This sets the pointer register so that the following two bytes write to the config register
				  writeBuf[1] = 0xD3;   	// This sets the 8 MSBs of the config register (bits 15-8) to 11000011
				  writeBuf[2] = 0x03;  		// This sets the 8 LSBs of the config register (bits 7-0) to 00000011
				  //std::cout <<  (unsigned)writeBuf[0] << " " << (unsigned) writeBuf[1] << " " << (unsigned)writeBuf[2] << std::endl;
	
				  // Initialize the buffer used to read data from the ADS1115 to 0
				  readBuf[0]= 0;		
				  readBuf[1]= 0;
				  //printf("Read buf FIRST: %u \n",readBuf);
					  
				  // Write writeBuf to the ADS1115, the 3 specifies the number of bytes we are writing,
					//printf("Write Statement...\n");
				  // this begins a single conversion
				  ::write(I2CFile, writeBuf, 3);	

				  // Wait for the conversion to complete, this requires bit 15 to change from 0->1
				 while ((readBuf[0] & 0x80) == 0 && maxIter < 10)	// readBuf[0] contains 8 MSBs of config register, AND with 10000000 to select bit 15
				 {
					  //printf("Read Statement...\n");
					  ::read(I2CFile, readBuf, 2);	// Read the config register into readBuf
					  //printf("Read buf1: %u \n",(unsigned) readBuf[0]);
					  //printf("Read buf2: %d \n",readBuf[1]);
					  usleep(10000);
					  maxIter++;
					  //printf("Iteration: %d \n", maxIter);
					  	
				}
				maxIter =0;
		
				  writeBuf[0] = 0;					// Set pointer register to 0 to read from the conversion register
					//printf("Write Statement...\n");
				  ::write(I2CFile, writeBuf, 1);
				  //printf("Read Statement...\n");
				  ::read(I2CFile, readBuf, 2);		// Read the contents of the conversion register into readBuf

				  val2 = readBuf[0] << 8 | readBuf[1];	// Combine the two bytes of readBuf into a single 16 bit result 

				  printf("A1 ++++ Voltage Reading %f (V) \n", (float)val2*4.096/32767.0);	

					if ((float)val*4.096/32767.0 > 1.5) {
						dirPart = 'R';
						usec = 50000;
					}
					else if ((float)val2*4.096/32767.0 > 1.5) {
						dirPart = 'L';
						usec = 50000;
					}
					else {
						dirPart = 'Q';
						usec = 0;
					}
				

				}
					//mutex.unlock();
				::close(I2CFile);
	    	}
};


QSSnake::QSSnake() : QMainWindow(0, 0) {
	resize(400, 200);
	setMinimumSize(400, 200);
	setMaximumSize(400, 200);
	setWindowTitle("QSSnake");
	statusBar()->showMessage("Welcome in QSSnake...");
	statusBar()->show();

	setStatusTip("QSSnake v0.1");

	QAction* new_game = new QAction("&New game", this);
	//new_game->setIcon(QIcon::fromTheme("media-playback-start")); error compling with fromTheme
	new_game->setStatusTip("Start new game");
	QAction* quit = new QAction("&Quit", this);
	//quit->setIcon(QIcon::fromTheme("application-exit"));
	quit->setStatusTip("Close game");

	QMenu* game = menuBar()->addMenu("&Game");
	game->addAction(new_game);
	game->addAction(quit);

	QToolBar* tool_bar = addToolBar("Main toolbar");
	tool_bar->addAction(new_game);
	tool_bar->addAction(quit);
	tool_bar->show();

	connect(quit, SIGNAL(triggered()), qApp, SLOT(quit()));
	connect(new_game, SIGNAL(triggered()), this, SLOT(startGame()));
	
	//Autostart

	QLabel* scoreLabel = new QLabel("");
	tool_bar->addSeparator();
	tool_bar->addWidget(scoreLabel);

	canvas = new Canvas(this, scoreLabel);
	setCentralWidget(canvas);

	
	Thread *thread = new Thread;
	thread->start();
	

	srand(time(NULL));
  	startGame();
}

void QSSnake::startGame() {
	canvas->initGame();
	canvas->startGame();
}

void QSSnake::keyPressEvent(QKeyEvent* event) {
	canvas->keyPressEvent(event);
}

QSSnake::Canvas::Canvas(QWidget* parent, QLabel* score_label) : QWidget(parent) {
	snake = NULL;
	timerId = -1;
	in_game = false;
	ignore_keys = false;
	bonus_in_game = false;
	score = 0;
	this->score_label = score_label;
	score_label->show();
}

void QSSnake::Canvas::initGame() {
	if(timerId > 0)
		killTimer(timerId);
	direction = 0x01;
	dots_size = 8;
	in_game = false;
	score = 0;
	ignore_keys = false;
	timerId = startTimer(80);
	bonus_in_game = false;
	bonus_direction = 0x00;
	snake_size = 1;
	max_dots = (width() / dots_size) * (height() * dots_size);
	updateScoreLabel();
	if(snake != NULL)
		delete[] snake;
	snake = new QPoint[max_dots];
	snake[0].setX(((width() / dots_size) / 2) * dots_size);
	snake[0].setY(((height() / dots_size) / 2) * dots_size);
	locateFood();
	//TimeStamp
	timeStamp = 0;
}

void QSSnake::Canvas::paintEvent(QPaintEvent* event) {
	if(!in_game)
		return;
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	for(int i = 0; i < snake_size; ++i) {
		if(i == 0) {
			painter.setBrush(QBrush("#FF0000"));
			painter.setPen(QColor("#FF0000"));
		} else {
			painter.setBrush(QBrush("#00AD01"));
			painter.setPen(QColor("#00AD01"));
		}
		painter.drawEllipse(snake[i].x(), snake[i].y(), dots_size, dots_size);
	}
	painter.setBrush(QBrush("#C7B100"));
	painter.setPen(QColor("#C7B100"));
	painter.drawEllipse(food.x(), food.y(), dots_size, dots_size);
	
	if(bonus_in_game) {
		painter.setBrush(QBrush("#00A1FF"));
		painter.setPen(QColor("#00A1FF"));
		painter.drawEllipse(bonus.x(), bonus.y(), dots_size, dots_size);
	}

}

void QSSnake::Canvas::keyPressEvent(QKeyEvent* event) {
	if(ignore_keys)
		return;
	switch(event->key()) {
		case Qt::Key_Up: {
			if(direction == 0x04 && snake_size > 1)
				return;
			direction = 0x01;	
			ignore_keys = true;
		}
		break;
		case Qt::Key_Right: {
			if(direction == 0x08 && snake_size > 1)
				return;
			direction = 0x02;
			ignore_keys = true;
		}
		break;
		case Qt::Key_Down: {
			if(direction == 0x01 && snake_size > 1)
				return;
			direction = 0x04;
			ignore_keys = true;
		}
		break;
		case Qt::Key_Left: {
			if(direction == 0x02 && snake_size > 1)
				return;
			direction = 0x08;
			ignore_keys = true;
		}
		break;
	}
}

void QSSnake::Canvas::timerEvent(QTimerEvent* event) {
	checkDirectionFile();
	moveSnake();
	checkCollisions();
	ignore_keys = false;
	if(score > 30 && direction != 0 && !bonus_in_game) {
		if(rand() % 250 == 5) {
			bonus_in_game = true;
			initBonus();
		}
	} else {
		if(direction != 0)
			moveBonus();
	}
	updateScoreLabel();
	repaint();
}

//====================================================================
void QSSnake::Canvas::checkDirectionFile() {
	//int dirPart = 'F';	
	//
	double compSec;
	time_t compT;
    
// Print the result to terminal, first convert from binary value to mV
	 //std::cout << "A0 --- Val: "<< val <<  "Voltage Reading (V) " << val*4.096/32767.0 << std::endl;
	 
	  // These three bytes are written to the ADS1115 to set the config register and start a conversion 
	 /* writeBuf[0] = 1;			// This sets the pointer register so that the following two bytes write to the config register
	  writeBuf[1] = 0xD3;   	// This sets the 8 MSBs of the config register (bits 15-8) to 11000011
	  writeBuf[2] = 0x03;  		// This sets the 8 LSBs of the config register (bits 7-0) to 00000011
	  
	  // Initialize the buffer used to read data from the ADS1115 to 0
	  readBuf[0]= 0;		
	  readBuf[1]= 0;
	  //printf("Read buf FIRST: %u \n",readBuf);
		  
	  // Write writeBuf to the ADS1115, the 3 specifies the number of bytes we are writing,
	  // this begins a single conversion
	  ::write(I2CFile, writeBuf, 3);	

	  // Wait for the conversion to complete, this requires bit 15 to change from 0->1
	 while ((readBuf[0] & 0x80) == 0 && maxIter < 3)	// readBuf[0] contains 8 MSBs of config register, AND with 10000000 to select bit 15
	 {
		  ::read(I2CFile, readBuf, 2);	// Read the config register into readBuf
		  //printf("Read buf2: %u \n",readBuf);
		  //sleep(1);
		  maxIter++;
		  printf("Iteration: %d \n", maxIter);
		  	
	 }

	  writeBuf[0] = 0;					// Set pointer register to 0 to read from the conversion register
	  ::write(I2CFile, writeBuf, 1);
	  
	  ::read(I2CFile, readBuf, 2);		// Read the contents of the conversion register into readBuf

	  val2 = readBuf[0] << 8 | readBuf[1];	// Combine the two bytes of readBuf into a single 16 bit result 
	  
	  printf("A1 --- Val: %d, Voltage Reading%f (V) \n", val2,  (float)val2*4.096/32767.0);	// Print the result to terminal, first convert from binary value to mV
          
		if ((float)val*4.096/32767.0 > 1) 
			dirPart = 'L';
		else if ((float)val2*4.096/32767.0 > 1)
			dirPart = 'R';	

	qDebug() << dirPart;*/
        //::close(I2CFile);
  


    /*	
    QFile file("/dev/mygpio");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
   // printf("Open I2C... \n");*/
    //QTextStream in(&file);
    //QString line = in.readLine();
    //long tsPart = line.split(" ")[0].toLong();
   // QString dirPart = line.split(" ")[1];
	//printf("Jiffies: %u\n", tsPart);
	//printf("Direction: %c\n", dirPart);
//	qDebug() << dirPart;
//	qDebug() << tsPart;
//	qDebug() << timeStamp;
	//printf("Time Stamp: %u\n", timeStamp);

    
/*Aboslute Directions
    if(line == "R"){
		if(direction == 0x04 && snake_size > 1)
		return;
		direction = 0x01;
		ignore_keys = true;
	}
	else if(line == "R"){
		if(direction == 0x08 && snake_size > 1)
		return;
		direction = 0x02;
		ignore_keys = true;
	}
	else if(line == "D"){
		if(direction == 0x01 && snake_size > 1)
		return;
		direction = 0x04;
		ignore_keys = true;
	}
	else if(line =="L"){
		if(direction == 0x02 && snake_size > 1)
		return;
		direction = 0x08;
		ignore_keys = true;
	}*/

//Relative to Absolute Directions
if(1){
	//timeStamp = tsPart; /*update timeStamp*/
    if(direction == 0x01 && dirPart == 'R'){
		direction = 0x02;	
		ignore_keys = true;
	}
	else if(direction == 0x02 && dirPart == 'R'){
		direction = 0x04;
		ignore_keys = true;
	}
	else if(direction == 0x04 && dirPart == 'R'){
		direction = 0x08;
		ignore_keys = true;
	}
	else if(direction == 0x08 && dirPart == 'R'){
		direction = 0x01;
		ignore_keys = true;
	}
	else if(direction == 0x01 && dirPart == 'L'){
		direction = 0x08;	
		ignore_keys = true;
	}
	else if(direction == 0x02 && dirPart == 'L'){
		direction = 0x01;
		ignore_keys = true;
	}
	else if(direction == 0x04 && dirPart == 'L'){
		direction = 0x02;
		ignore_keys = true;
	}
	else if(direction == 0x08 && dirPart == 'L'){
		direction = 0x04;
		ignore_keys = true;
	}
	dirPart = 'Q';
}
}

//=====================================================================

void QSSnake::Canvas::moveSnake() {
	if(direction == 0)
		return;
	for(int i = snake_size - 1; i > 0; --i) {
		snake[i].setX(snake[i - 1].x());
		snake[i].setY(snake[i - 1].y());
	}
	switch(direction) {
		case 0x01:
			snake[0] += QPoint(0, -1 * dots_size);
		break;
		case 0x02:
			snake[0] += QPoint(dots_size, 0);
		break;
		case 0x04:
			snake[0] += QPoint(0, dots_size);
		break;
		case 0x08:
			snake[0] += QPoint(-1 * dots_size, 0);
		break;
	}
	if(snake[0].x() >= width())
		snake[0].setX(0);
	if(snake[0].x() < 0)
		snake[0].setX((width() / dots_size) * dots_size);
	if(snake[0].y() >= height())
		snake[0].setY(0);
	if(snake[0].y() < 0)
		snake[0].setY((height() / dots_size) * dots_size);
}

void QSSnake::Canvas::startGame() {
	in_game = true;
}

void QSSnake::Canvas::locateFood() {
	food.setX((rand() % (width() / dots_size)) * dots_size);
	food.setY((rand() % (height() / dots_size)) * dots_size);
}

void QSSnake::Canvas::checkCollisions() {
	if(snake[0] == food) {
		snake[snake_size] = QPoint(-100, -100);
		++snake_size;
		score += 2;
		locateFood();
	}
	if(snakeCollision(snake[0], true)) {
		initGame();
		in_game = true;
	}
	if(snake[0] == bonus) {
		bonus_in_game = false;
		score += 10;
		for(int i = snake_size; i < snake_size + 5; ++i) {
			snake[i] = QPoint(-100, -100);
		}
		snake_size += 5;
	}
}

void QSSnake::Canvas::moveBonus() {	
	if(rand() % 5 == 1)
		bonus_direction = 0;
	QPoint tmp_pos = bonus;
	while(true) {
		if(bonus_direction == 0) {
			while(bonus_direction == 0 || bonus_direction % 2 != 0) {
				bonus_direction = rand() % 8 + 1;
				if(bonus_direction == 1)
					break;
			}
		}
		switch(bonus_direction) {
			case 0x01: {
				bonus += QPoint(0, -1 * dots_size);
			}
			break;
			case 0x02: {
				bonus += QPoint(dots_size, 0);
			}
			break;
			case 0x04: {
				bonus += QPoint(0, dots_size);
			}
			break;
			case 0x08: {
				bonus += QPoint(-1 * dots_size, 0);
			}
			break;
		}
		if(snakeCollision(bonus, true)) {
			bonus_direction = 0;
			bonus = tmp_pos;
			continue;
		}
		if(bonus.x() >= width())
			bonus.setX(0);
		if(bonus.x() < 0)
			bonus.setX((width() / dots_size) * dots_size);
		if(bonus.y() >= height())
			bonus.setY(0);
		if(bonus.y() < 0)
			bonus.setY((height() / dots_size) * dots_size);
		break;
	}
}

void QSSnake::Canvas::initBonus() {
	bonus = snake[0];
	bonus_direction = 0;
	while(snakeCollision(bonus)) {
		bonus.setX((rand() % (width() / dots_size)) * dots_size);
		bonus.setY((rand() % (height() / dots_size)) * dots_size);
	}
}

bool QSSnake::Canvas::snakeCollision(const QPoint& p, bool skip_head) const {
	for(int i = (skip_head ? 1 : 0); i < snake_size; ++i) {
		if(p == snake[i])
			return true;
	}
	return false;
}

void QSSnake::Canvas::updateScoreLabel() {
	score_label->setText(QString("Score: ") + QString::number(score));
}
