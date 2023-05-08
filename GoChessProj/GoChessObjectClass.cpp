class GoChessObjectClass {
	
	private:
		// Black: true; White: false;
		bool isBlack;
		int xPos;
		int yPos;

	public:
		GoChessObjectClass(bool isBlackValue, int xPosValue, int yPosValue) {
			isBlack = isBlackValue;
			xPos = xPosValue;
			yPos = yPosValue;
		}
		
		void setIsBlack(bool value) {
			isBlack = value;
		}

		bool getIsBlack() {
			return isBlack;
		}

		void setXPos(int value) {
			xPos = value;
		}

		int getXPos() {
			return xPos;
		}

		void setYPos(int value) {
			yPos = value;
		}

		int getYPos() {
			return yPos;
		}


};