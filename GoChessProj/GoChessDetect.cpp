#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include "GoChessDetect.h"



using namespace cv;
using namespace std;
/// Global variables



	Mat src, src_gray;
	Mat dst, cdst, dst_erode;
	Mat warp, warp_dst, warp_cdst, warp_hsv;
	Mat black_range, white_range;

	int edgeThresh = 1;
	int lowThreshold = 30;
	int const max_lowThreshold = 100;
	double thersoldRatio = 3.0;
	int kernel_size = 3;

	vector<Point> chess;
	vector<vector<Point>> black_pieces, white_pieces;

	Vec2f lines_hori[19];
	Vec2f lines_vert[19];
	Point2f intercept[19][19];
	bool checkPieceExist[19][19];


	//Line Reconstruction
	void drawLine(float rho, float theta, int num) {
		Point pt1, pt2;
		Scalar red = Scalar(0, 0, 255);
		Scalar green = Scalar(0, 255, 0);
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		if (num == 0) line(warp, pt1, pt2, red, 2, LINE_AA);
		else line(warp, pt1, pt2, green, 2, LINE_AA);
	}

	//Detect Chessboard
	void chessDetect() {
		Canny(src_gray, dst, lowThreshold, lowThreshold * thersoldRatio, kernel_size);

		cvtColor(dst, cdst, cv::COLOR_GRAY2BGR);

		vector<vector<Point>> contours;
		int max_area = 0, max_cont = 0;
		findContours(dst, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

		for (size_t i = 0; i < contours.size(); i++) {
			double area = contourArea(contours[i]);  //  Find the area of contour
			int perim = arcLength(contours[i], true);
			vector<Point> approx;
			approxPolyDP(contours[i], approx, 0.02 * perim, true);
			if (area > max_area && approx.size() == 4) {
				max_area = area;
				chess = approx;
				max_cont = i;//Store the index of largest contour
			}
		}
		drawContours(cdst, contours, max_cont, Scalar(0, 0, 255), 2, LINE_AA);
		resize(cdst, cdst, Size(src.cols / 1.5, src.rows / 1.5));

	}

	//Rectify Chessboard
	void warpChess() {
		Point tl, tr, bl, br;
		int small_sum = INT_MAX, large_sum = INT_MIN, small_diff = INT_MAX, large_diff = INT_MIN;
		Point center(0, 0);

		for (int i = 0; i < chess.size(); i++) center += chess[i];

		center *= (1. / chess.size());

		vector<Point> top, bot;

		for (int i = 0; i < chess.size(); i++)
		{
			if (chess[i].y < center.y)
				top.push_back(chess[i]);
			else
				bot.push_back(chess[i]);
		}

		tl = top[0].x > top[1].x ? top[1] : top[0];
		tr = top[0].x > top[1].x ? top[0] : top[1];
		bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
		br = bot[0].x > bot[1].x ? bot[0] : bot[1];

		warp = Mat::zeros(950, 890, CV_8SC3);
		vector<Point2f> orig_pt, warp_pt;

		orig_pt.push_back(tl);
		orig_pt.push_back(tr);
		orig_pt.push_back(br);
		orig_pt.push_back(bl);


		warp_pt.push_back(Point2f(0, 0));
		warp_pt.push_back(Point2f(warp.cols, 0));
		warp_pt.push_back(Point2f(warp.cols, warp.rows));
		warp_pt.push_back(Point2f(0, warp.rows));

		Mat tranmtx = getPerspectiveTransform(orig_pt, warp_pt);
		warpPerspective(src, warp, tranmtx, warp.size());
	}

	//Highlight piece colour
	void pieceColour() {
		cvtColor(warp, warp_hsv, cv::COLOR_BGR2HSV);
		Canny(warp, warp_dst, lowThreshold, lowThreshold * thersoldRatio, kernel_size);
		cvtColor(warp_dst, warp_cdst, cv::COLOR_GRAY2BGR);
		inRange(warp_hsv, cv::Scalar(0, 0, 0), cv::Scalar(180, 255, 80), black_range);
		inRange(warp_hsv, cv::Scalar(0, 0, 153), cv::Scalar(180, 80, 255), white_range);
		Mat element_cross = getStructuringElement(MORPH_CROSS, Size(3, 3));
		Mat element_ellip = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
		Mat element_rect = getStructuringElement(MORPH_RECT, Size(9, 9));

		erode(black_range, black_range, element_cross);
		erode(black_range, black_range, element_ellip);
		erode(black_range, black_range, element_ellip);
		erode(black_range, black_range, element_ellip);
		erode(black_range, black_range, element_rect);
		dilate(black_range, black_range, element_rect);
		dilate(black_range, black_range, element_ellip);

		erode(white_range, white_range, element_ellip);
		erode(white_range, white_range, element_ellip);
		erode(white_range, white_range, element_ellip);
		erode(white_range, white_range, element_rect);
		dilate(white_range, white_range, element_rect);
		dilate(white_range, white_range, element_ellip);

	}

	//Line Detection
	void lineDetect() {
		vector<Vec2f> lines;
		HoughLines(warp_dst, lines, 1, CV_PI / 180, 220, 0, 0);

		vector<vector<Vec2f>> lines_gp;

		for (size_t i = 0; i < lines.size(); i++) {
			float rho = lines[i][0], theta = lines[i][1];
			bool check = false;
			for (size_t j = 0; j < lines_gp.size(); j++) {
				for (size_t k = 0; k < lines_gp[j].size(); k++) {
					float rho_c = lines_gp[j][k][0], theta_c = lines_gp[j][k][1];
					if ((abs(theta - theta_c) <= 0.05 && abs(rho - rho_c) <= 15) && !(rho == rho_c && theta == theta_c)) {
						check = true;
						break;
					}
				}
				if (check) {
					lines_gp[j].push_back(lines[i]);
					break;
				}
			}
			if (!check) {
				vector<Vec2f> newColumn;
				newColumn.push_back(lines[i]);
				lines_gp.push_back(newColumn);
			}
			else check = false;


		}


		bool hori_elem[19] = { false }, vert_elem[19] = { false };
		double e_xspace = 890 / (4.2 + 2.4 * 18) * 2.1, g_xspace = 890 / (4.2 + 2.4 * 18) * 2.4;
		double e_yspace = 950 / (4.2 + 2.4 * 18) * 2.1, g_yspace = 950 / (4.2 + 2.4 * 18) * 2.4;

		RNG rng(12345);
		for (size_t i = 0; i < lines_gp.size(); i++)
		{
			if (lines_gp[i].size() > 1) {
				float rho_t = 0, theta_t = 0;
				for (size_t j = 0; j < lines_gp[i].size(); j++) {
					rho_t += lines_gp[i][j][0];
					theta_t += lines_gp[i][j][1];
				}
				float rho = rho_t / lines_gp[i].size(), theta = theta_t / lines_gp[i].size();
				Vec2f lines(rho, theta);
				Point pt1, pt2;
				double a = cos(theta), b = sin(theta);
				double x0 = a * rho, y0 = b * rho;
				pt1.x = cvRound(x0 + 1000 * (-b));
				pt1.y = cvRound(y0 + 1000 * (a));
				pt2.x = cvRound(x0 - 1000 * (-b));
				pt2.y = cvRound(y0 - 1000 * (a));
				if (theta > 1.56 && theta < 1.58) {
					int y = (pt1.y + pt2.y) / 2;
					lines_hori[(int)(ceil(((double)(y)-e_yspace - (g_yspace / 2)) / g_yspace))] = lines;
					hori_elem[(int)(ceil(((double)(y)-e_yspace - (g_yspace / 2)) / g_yspace))] = true;
					drawLine(rho, theta, 1);
				}
				else if (theta == 0) {
					int x = (pt1.x + pt2.x) / 2;
					lines_vert[(int)(ceil(((double)(x)-e_xspace - (g_xspace / 2)) / g_xspace))] = lines;
					vert_elem[(int)(ceil(((double)(x)-e_xspace - (g_xspace / 2)) / g_xspace))] = true;
					drawLine(rho, theta, 1);
				}
			}
		}

		for (int i = 0; i < 19; i++) {
			if (!hori_elem[i]) {
				int y = e_yspace + g_yspace * i;
				Vec2f nl(y * sin(1.570796327), 1.57079);
				lines_hori[i] = nl;
				hori_elem[i] = true;
				drawLine(nl[0], nl[1], 0);
			}
			if (!vert_elem[i]) {
				int x = e_xspace + g_xspace * i;
				Vec2f nl(x * cos(0), 0);
				lines_vert[i] = nl;
				vert_elem[i] = true;
				drawLine(nl[0], nl[1], 0);
			}
		}

	}

	void pointDetect(std::vector<GoChessObjectClass> &goChessList ) {
		int count = 0;
		for (int i = 0; i < 19; i++) {
			Point2f i1, i2;
			double i_a = cos(lines_hori[i][1]), i_b = sin(lines_hori[i][1]);
			double i_x0 = i_a * lines_hori[i][0], i_y0 = i_b * lines_hori[i][0];
			i1.x = cvRound(i_x0 + 1000 * (-i_b));
			i1.y = cvRound(i_y0 + 1000 * (i_a));
			i2.x = cvRound(i_x0 - 1000 * (-i_b));
			i2.y = cvRound(i_y0 - 1000 * (i_a));
			for (int j = 0; j < 19; j++) {
				Point2f j1, j2;
				double j_a = cos(lines_vert[j][1]), j_b = sin(lines_vert[j][1]);
				double j_x0 = j_a * lines_vert[j][0], j_y0 = j_b * lines_vert[j][0];
				j1.x = cvRound(j_x0 + 1000 * (-j_b));
				j1.y = cvRound(j_y0 + 1000 * (j_a));
				j2.x = cvRound(j_x0 - 1000 * (-j_b));
				j2.y = cvRound(j_y0 - 1000 * (j_a));
				Point2f x = j1 - i1;
				Point2f d1 = i2 - i1;
				Point2f d2 = j2 - j1;
				float cross = (float)(d1.x * d2.y - d1.y * d2.x);

				double t1 = (x.x * d2.y - x.y * d2.x) / cross;
				intercept[i][j] = i1 + d1 * t1;
				for (int k = 0; k < black_pieces.size(); k++) {
					if (pointPolygonTest(black_pieces[k], intercept[i][j], false) > 0) {
						//cout << "b:[" << j+1 << "," << i+1 << "]" << endl;

						if (!checkPieceExist[i][j]) {
							GoChessObjectClass newChess(true, i, j);
							goChessList.emplace_back(newChess);
							checkPieceExist[i][j] = true;
							count++;
						}
												
					}
				}
				for (int k = 0; k < white_pieces.size(); k++) {
					if (pointPolygonTest(white_pieces[k], intercept[i][j], false) > 0) {
						//cout << "w:[" << j+1 << "," << i+1 << "]" << endl;

						if (!checkPieceExist[i][j]) {
							GoChessObjectClass newChess(false, i, j);
							goChessList.emplace_back(newChess);
							checkPieceExist[i][j] = true;
							count++;
						}

					}
				}

			}
		}
		cout << "Detected Piece: " << black_pieces.size() + white_pieces.size() << endl;
		cout << "Located Piece: " << count << endl;
	}

	// Black and White Pieces Grouping
	void pieceDetect() {
		vector<vector<Point>> black_contours;
		findContours(black_range, black_contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
		vector<vector<Point>> black_contours_poly(black_contours.size());
		vector<Point2f>black_center(black_contours.size());
		vector<float>black_radius(black_contours.size());
		int black_no = 0;
		for (int i = 0; i < black_contours.size(); i++) {
			int perim = arcLength(black_contours[i], true);
			double area = contourArea(black_contours[i]);
			approxPolyDP(black_contours[i], black_contours_poly[i], 0.02 * perim, true);
			if (area >= 200 && area <= 50000) {
				minEnclosingCircle(black_contours_poly[i], black_center[black_no], black_radius[black_no]);
				circle(warp, black_center[black_no], (int)black_radius[black_no], Scalar(0, 0, 255), 2, 8, 0);
				black_no++;
				black_pieces.push_back(black_contours_poly[i]);
			}
		}
		vector<vector<Point>> white_contours;
		findContours(white_range, white_contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
		vector<vector<Point>> white_contours_poly(white_contours.size());
		vector<Point2f>white_center(white_contours.size());
		vector<float>white_radius(white_contours.size());
		int white_no = 0;
		for (int i = 0; i < white_contours.size(); i++) {
			int perim = arcLength(white_contours[i], true);
			double area = contourArea(white_contours[i]);
			approxPolyDP(white_contours[i], white_contours_poly[i], 0.02 * perim, true);
			if (area >= 200 && area <= 50000) {
				minEnclosingCircle(white_contours_poly[i], white_center[white_no], white_radius[white_no]);
				circle(warp, white_center[white_no], (int)white_radius[white_no], Scalar(0, 255, 0), 2, 8, 0);
				white_no++;
				white_pieces.push_back(white_contours_poly[i]);
			}
		}
	}

	void pieceDetectHough() {
		GaussianBlur(black_range, black_range, cv::Size(9, 9), 2, 2);
		vector<Vec3f> circles;
		HoughCircles(black_range, circles, HOUGH_GRADIENT, 1, black_range.rows / 8, 100, 20, 0, 0);
		for (size_t current_circle = 0; current_circle < circles.size(); ++current_circle) {
			Point center(cvRound(circles[current_circle][0]), cvRound(circles[current_circle][1]));
			int radius = cvRound(circles[current_circle][2]);

			circle(warp, center, radius, cv::Scalar(0, 255, 0), 5);
		}
	}

	
	void initGoBoard() {

		src = NULL;
		src_gray = NULL;
		dst = NULL;
		cdst = NULL;
		dst_erode = NULL;
		warp = NULL;
		warp_dst = NULL;
		warp_cdst = NULL;
		warp_hsv = NULL;
		black_range = NULL;
		white_range = NULL;

		chess.clear();
		black_pieces.clear();
		white_pieces.clear();

		lines_hori[19] = {};
		lines_vert[19] = {};
		intercept[19][19] = {};
		
		
		int rows = sizeof(checkPieceExist) / sizeof(checkPieceExist[0]);
		int cols = sizeof(checkPieceExist[0]) / sizeof(checkPieceExist[0][0]);

		for (int i = 0; i < rows; i++)
			for (int j = 0; j < cols; j++)
				checkPieceExist[i][j] = false;


	}

	/*

	void outputGoBoard(char goBoard[][19]) {
		// Get the dimensions of the 2D array.
		int numRows = sizeof(goBoard) / sizeof(goBoard[0]);
		int numCols = sizeof(goBoard[0]) / sizeof(goBoard[0][0]);

		// Loop through the 2D array and output its elements.
		for (int row = 0; row < numRows; ++row) {
			for (int col = 0; col < numCols; ++col) {
				std::cout << goBoard[row][col] << ' ';
			}
			std::cout << std::endl;
		}
	}
	*/


	int GoChessDetect::goChessDetect(std::string data, std::vector<GoChessObjectClass> &goChessList)
	{
		src = NULL;
		initGoBoard();

		if (data.empty()) {
			return 1;
		}

		/// Load an image
		std::vector<uchar> vectorData(data.begin(), data.end());

		src = cv::imdecode(vectorData, IMREAD_UNCHANGED);


		if (!src.data)
		{
			return -1;
		}

		/// Convert the image to grayscale
		cvtColor(src, src_gray, cv::COLOR_BGR2GRAY);


		/// Create a window
		//namedWindow( window_name, WINDOW_AUTOSIZE );

		/// Reduce noise with a kernel 3j1.x
		blur(src_gray, src_gray, Size(3, 3));


		/// Canny detector
		chessDetect();

		warpChess();

		pieceColour();

		lineDetect();

		pieceDetect();

		pointDetect(goChessList);	

		//outputGoBoard(arr);

		return 0;
	}
