# Go Chessboard and Piece Recognition API

This is a REST API project built using C++, [OpenCV](https://opencv.org/) and [Crow](https://github.com/crowcpp/crow) framework. 

It is able to detect the Go chessboard and its game composition.

The API accepts a Base64-encoded image input, then returns the color and the relative [x,y] position of Go chess pieces on the chessboard.

## Demo

Website Demo: https://heihei4c.github.io/GoChessProjWeb/

API Demo: https://api.heihei4c.dev/goChessDetectAPI

## Installation

1. Clone the repository:

```
https://github.com/heihei4c/goChessDetect.git
```

2. Install the necessary dependencies using vcpkg:

```
vcpkg install opencv
vcpkg install crow
vcpkg integrate install
```

3. Build and run the API in [Visual Studio](https://visualstudio.microsoft.com/)


## Usage

The API listens on https by default. To send a request to the API, you can use a tool such as `curl` or [Postman](https://www.postman.com/downloads/):

```
curl -X POST https://localhost/goChessDetectAPI -H "Content-Type: text/plain" --data "${BASE64_IMAGE_DATA}"
```

Replace `BASE64_IMAGE_DATA` with the Base64-encoded image data you wish to send.

The API will respond with a JSON object containing an lis of a boolean variable (isBlack) and 2 integer variables (xPos, yPos):

```
[
    {
      "isBlack": true,
      "xPos": 8,
      "yPos": 9
    },
    ...
    {
      "isBlack": false,
      "xPos": 2,
      "yPos": 6
    },
]

```

## License

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT). See the LICENSE file for details.

## Acknowledgments

This project was built using the following libraries:

- [OpenCV](https://opencv.org/)
- [Crow](https://github.com/crowcpp/crow)
