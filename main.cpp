#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <atomic>
#include <functional>
#include <memory>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

// ============================================================
// CONFIGURATION
// ============================================================

constexpr int PORT = 8080;

constexpr int WORKER_THREADS = 128;

constexpr int IOCP_THREADS = 4;

constexpr int LISTEN_BACKLOG = SOMAXCONN;

constexpr int BUFFER_SIZE = 8192;


// ============================================================
// THREAD POOL
// ============================================================

class ThreadPool {

private:

    std::vector<std::thread> workers;

    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;

    std::condition_variable condition;

    bool stopping = false;


public:

    ThreadPool(int threadCount) {

        for (int i = 0; i < threadCount; ++i) {

            workers.emplace_back([this]() {

                while (true) {

                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(
                            queueMutex
                        );

                        condition.wait(
                            lock,
                            [this]() {
                                return stopping ||
                                       !tasks.empty();
                            }
                        );

                        if (
                            stopping &&
                            tasks.empty()
                        ) {
                            return;
                        }

                        task =
                            std::move(tasks.front());

                        tasks.pop();
                    }

                    task();
                }
            });
        }
    }


    void enqueue(
        std::function<void()> task
    ) {

        {
            std::lock_guard<std::mutex> lock(
                queueMutex
            );

            if (stopping) {
                return;
            }

            tasks.push(
                std::move(task)
            );
        }

        condition.notify_one();
    }


    ~ThreadPool() {

        {
            std::lock_guard<std::mutex> lock(
                queueMutex
            );

            stopping = true;
        }

        condition.notify_all();

        for (auto& worker : workers) {

            if (worker.joinable()) {
                worker.join();
            }
        }
    }
};


// ============================================================
// MOVIE
// ============================================================

class Movie {

public:

    int id;

    std::string title;

    std::string language;

    int duration;


    Movie(
        int id,
        std::string title,
        std::string language,
        int duration
    )
        : id(id),
          title(std::move(title)),
          language(std::move(language)),
          duration(duration) {}
};


// ============================================================
// SEAT
// ============================================================

class Seat {

public:

    std::string number;

    std::string type;

    int price;

    bool booked;


    Seat() = default;


    Seat(
        std::string number,
        std::string type,
        int price
    )
        : number(std::move(number)),
          type(std::move(type)),
          price(price),
          booked(false) {}
};


// ============================================================
// SHOW
// ============================================================

class Show {

public:

    int id;

    int movieId;

    int screenId;

    std::string time;


    Show(
        int id,
        int movieId,
        int screenId,
        std::string time
    )
        : id(id),
          movieId(movieId),
          screenId(screenId),
          time(std::move(time)) {}
};


// ============================================================
// BOOKING
// ============================================================

class Booking {

public:

    int id;

    std::string seat;

    int amount;


    Booking(
        int id,
        std::string seat,
        int amount
    )
        : id(id),
          seat(std::move(seat)),
          amount(amount) {}
};


// ============================================================
// BOOKING SERVICE
// ============================================================

class BookingService {

private:

    std::unordered_map<
        std::string,
        Seat
    > seats;

    std::vector<Booking> bookings;

    std::mutex bookingMutex;

    std::atomic<int> nextBookingId{1};


public:

    BookingService() {

        // ----------------------------------------------------
        // SILVER
        // ----------------------------------------------------

        for (int i = 1; i <= 34; ++i) {

            std::string seat =
                "A" + std::to_string(i);

            seats.emplace(
                seat,
                Seat(
                    seat,
                    "Silver",
                    150
                )
            );
        }


        // ----------------------------------------------------
        // GOLD
        // ----------------------------------------------------

        for (int i = 35; i <= 67; ++i) {

            std::string seat =
                "A" + std::to_string(i);

            seats.emplace(
                seat,
                Seat(
                    seat,
                    "Gold",
                    250
                )
            );
        }


        // ----------------------------------------------------
        // PLATINUM
        // ----------------------------------------------------

        for (int i = 68; i <= 100; ++i) {

            std::string seat =
                "A" + std::to_string(i);

            seats.emplace(
                seat,
                Seat(
                    seat,
                    "Platinum",
                    400
                )
            );
        }
    }


    // ========================================================
    // BOOK SEAT
    // ========================================================

    std::string bookSeat(
        const std::string& seatNumber
    ) {

        std::lock_guard<std::mutex> lock(
            bookingMutex
        );


        auto it =
            seats.find(seatNumber);


        if (it == seats.end()) {

            return
                "404|"
                "{\"error\":\"Seat not found\"}";
        }


        Seat& seat =
            it->second;


        // ----------------------------------------------------
        // CRITICAL SECTION
        // ----------------------------------------------------

        if (seat.booked) {

            return
                "409|"
                "{\"error\":\"Seat already booked\"}";
        }


        // Check + mark happens atomically
        // because both operations are protected
        // by the same mutex.

        seat.booked = true;


        int bookingId =
            nextBookingId.fetch_add(1);


        bookings.emplace_back(
            bookingId,
            seatNumber,
            seat.price
        );


        std::ostringstream json;


        json
            << "{"
            << "\"bookingId\":"
            << bookingId
            << ","
            << "\"seat\":\""
            << seatNumber
            << "\","
            << "\"type\":\""
            << seat.type
            << "\","
            << "\"amount\":"
            << seat.price
            << "}";


        return
            "201|" +
            json.str();
    }


    // ========================================================
    // GET SEATS
    // ========================================================

    std::string getSeats() {

        std::lock_guard<std::mutex> lock(
            bookingMutex
        );


        std::ostringstream json;

        json << "[";


        bool first = true;


        for (
            const auto& item :
            seats
        ) {

            const Seat& seat =
                item.second;


            if (!first) {
                json << ",";
            }

            first = false;


            json
                << "{"
                << "\"seat\":\""
                << seat.number
                << "\","
                << "\"type\":\""
                << seat.type
                << "\","
                << "\"price\":"
                << seat.price
                << ","
                << "\"status\":\""
                << (
                    seat.booked
                    ? "BOOKED"
                    : "AVAILABLE"
                )
                << "\""
                << "}";
        }


        json << "]";


        return json.str();
    }
};


// ============================================================
// GLOBAL DATA
// ============================================================

BookingService bookingService;


std::vector<Movie> movies = {

    Movie(
        1,
        "Interstellar",
        "English",
        169
    ),

    Movie(
        2,
        "Inception",
        "English",
        148
    ),

    Movie(
        3,
        "3 Idiots",
        "Hindi",
        170
    )
};


std::vector<Show> shows = {

    Show(
        1,
        1,
        1,
        "10:00"
    ),

    Show(
        2,
        1,
        1,
        "14:00"
    ),

    Show(
        3,
        2,
        2,
        "18:00"
    ),

    Show(
        4,
        3,
        3,
        "21:00"
    )
};


// ============================================================
// IOCP CONNECTION
// ============================================================

struct Connection {

    SOCKET socket;

    HANDLE iocp;

    char receiveBuffer[BUFFER_SIZE];

    std::string requestBuffer;

    std::string sendBuffer;

    WSABUF receiveWSABUF;

    WSABUF sendWSABUF;

    OVERLAPPED receiveOverlapped{};

    OVERLAPPED sendOverlapped{};

    std::atomic<bool> closed{false};

    std::atomic<bool> sendPending{false};


    Connection(
        SOCKET socket,
        HANDLE iocp
    )
        : socket(socket),
          iocp(iocp) {

        receiveWSABUF.buf =
            receiveBuffer;

        receiveWSABUF.len =
            BUFFER_SIZE;
    }
};


// ============================================================
// CLOSE CONNECTION
// ============================================================

void closeConnection(
    Connection* connection
) {

    if (connection == nullptr) {
        return;
    }


    bool expected = false;


    if (
        !connection->closed.compare_exchange_strong(
            expected,
            true
        )
    ) {

        return;
    }


    shutdown(
        connection->socket,
        SD_BOTH
    );


    closesocket(
        connection->socket
    );
}


// ============================================================
// SEND RESPONSE
// ============================================================

void sendResponse(
    Connection* connection,
    int statusCode,
    const std::string& body
) {

    if (
        connection->closed.load()
    ) {
        return;
    }


    std::string statusText;


    switch (statusCode) {

        case 200:
            statusText = "OK";
            break;

        case 201:
            statusText = "Created";
            break;

        case 400:
            statusText = "Bad Request";
            break;

        case 404:
            statusText = "Not Found";
            break;

        case 409:
            statusText = "Conflict";
            break;

        default:
            statusText = "Internal Server Error";
            break;
    }


    std::ostringstream response;


    response
        << "HTTP/1.1 "
        << statusCode
        << " "
        << statusText
        << "\r\n";


    response
        << "Content-Type: application/json\r\n";


    response
        << "Content-Length: "
        << body.size()
        << "\r\n";


    response
        << "Connection: keep-alive\r\n";


    response
        << "Keep-Alive: timeout=30\r\n";


    response
        << "\r\n";


    response
        << body;


    connection->sendBuffer =
        response.str();


    connection->sendWSABUF.buf =
        connection->sendBuffer.data();


    connection->sendWSABUF.len =
        static_cast<ULONG>(
            connection->sendBuffer.size()
        );


    connection->sendPending.store(
        true
    );


    DWORD bytesSent = 0;


    int result =
        WSASend(
            connection->socket,
            &connection->sendWSABUF,
            1,
            &bytesSent,
            0,
            &connection->sendOverlapped,
            nullptr
        );


    if (
        result == SOCKET_ERROR
    ) {

        int error =
            WSAGetLastError();


        if (
            error != WSA_IO_PENDING
        ) {

            connection->sendPending.store(
                false
            );

            closeConnection(
                connection
            );
        }
    }
}


// ============================================================
// ROUTER
// ============================================================

void processRequest(
    Connection* connection,
    const std::string& request
) {

    std::istringstream stream(
        request
    );


    std::string method;

    std::string path;

    std::string version;


    stream
        >> method
        >> path
        >> version;


    // ========================================================
    // HEALTH
    // ========================================================

    if (
        method == "GET" &&
        path == "/health"
    ) {

        sendResponse(
            connection,
            200,
            "{\"status\":\"UP\"}"
        );

        return;
    }


    // ========================================================
    // MOVIES
    // ========================================================

    if (
        method == "GET" &&
        path == "/movies"
    ) {

        std::ostringstream json;

        json << "[";


        for (
            size_t i = 0;
            i < movies.size();
            ++i
        ) {

            if (i > 0) {
                json << ",";
            }


            json
                << "{"
                << "\"id\":"
                << movies[i].id
                << ","
                << "\"title\":\""
                << movies[i].title
                << "\","
                << "\"language\":\""
                << movies[i].language
                << "\","
                << "\"duration\":"
                << movies[i].duration
                << "}";
        }


        json << "]";


        sendResponse(
            connection,
            200,
            json.str()
        );

        return;
    }


    // ========================================================
    // SHOWS
    // ========================================================

    if (
        method == "GET" &&
        path == "/shows"
    ) {

        std::ostringstream json;

        json << "[";


        for (
            size_t i = 0;
            i < shows.size();
            ++i
        ) {

            if (i > 0) {
                json << ",";
            }


            json
                << "{"
                << "\"id\":"
                << shows[i].id
                << ","
                << "\"movieId\":"
                << shows[i].movieId
                << ","
                << "\"screenId\":"
                << shows[i].screenId
                << ","
                << "\"time\":\""
                << shows[i].time
                << "\""
                << "}";
        }


        json << "]";


        sendResponse(
            connection,
            200,
            json.str()
        );

        return;
    }


    // ========================================================
    // SEATS
    // ========================================================

    if (
        method == "GET" &&
        path == "/seats"
    ) {

        sendResponse(
            connection,
            200,
            bookingService.getSeats()
        );

        return;
    }


    // ========================================================
    // BOOK
    // ========================================================

    if (
        method == "POST"
    ) {

        const std::string prefix =
            "/book?seat=";


        if (
            path.rfind(prefix, 0) == 0
        ) {

            std::string seatNumber =
                path.substr(
                    prefix.size()
                );


            size_t amp =
                seatNumber.find('&');


            if (
                amp != std::string::npos
            ) {

                seatNumber =
                    seatNumber.substr(
                        0,
                        amp
                    );
            }


            if (
                seatNumber.empty()
            ) {

                sendResponse(
                    connection,
                    400,
                    "{\"error\":\"Seat required\"}"
                );

                return;
            }


            std::string result =
                bookingService.bookSeat(
                    seatNumber
                );


            size_t separator =
                result.find('|');


            int statusCode =
                std::stoi(
                    result.substr(
                        0,
                        separator
                    )
                );


            std::string body =
                result.substr(
                    separator + 1
                );


            sendResponse(
                connection,
                statusCode,
                body
            );


            return;
        }
    }


    // ========================================================
    // 404
    // ========================================================

    sendResponse(
        connection,
        404,
        "{\"error\":\"Endpoint not found\"}"
    );
}


// ============================================================
// START RECEIVE
// ============================================================

void startReceive(
    Connection* connection
) {

    if (
        connection->closed.load()
    ) {
        return;
    }


    ZeroMemory(
        &connection->receiveOverlapped,
        sizeof(OVERLAPPED)
    );


    connection->receiveWSABUF.buf =
        connection->receiveBuffer;


    connection->receiveWSABUF.len =
        BUFFER_SIZE;


    DWORD flags = 0;

    DWORD bytesReceived = 0;


    int result =
        WSARecv(
            connection->socket,
            &connection->receiveWSABUF,
            1,
            &bytesReceived,
            &flags,
            &connection->receiveOverlapped,
            nullptr
        );


    if (
        result == SOCKET_ERROR
    ) {

        int error =
            WSAGetLastError();


        if (
            error != WSA_IO_PENDING
        ) {

            closeConnection(
                connection
            );
        }
    }
}


// ============================================================
// PROCESS COMPLETED HTTP REQUEST
// ============================================================

void handleReceiveCompletion(
    Connection* connection,
    DWORD bytesTransferred,
    ThreadPool& workers
) {

    if (
        bytesTransferred == 0
    ) {

        closeConnection(
            connection
        );

        return;
    }


    connection->requestBuffer.append(
        connection->receiveBuffer,
        bytesTransferred
    );


    // --------------------------------------------------------
    // Wait for complete HTTP headers.
    // --------------------------------------------------------

    size_t headerEnd =
        connection->requestBuffer.find(
            "\r\n\r\n"
        );


    if (
        headerEnd == std::string::npos
    ) {

        startReceive(
            connection
        );

        return;
    }


    // --------------------------------------------------------
    // Extract request.
    // --------------------------------------------------------

    std::string request =
        connection->requestBuffer;


    connection->requestBuffer.clear();


    // --------------------------------------------------------
    // IMPORTANT:
    //
    // We don't start another receive yet.
    //
    // The current request is processed by the worker pool.
    // After WSASend completes, the next receive begins.
    //
    // This prevents two requests on one connection from
    // writing responses simultaneously.
    // --------------------------------------------------------

    workers.enqueue(
        [
            connection,
            request
        ]() {

            if (
                connection->closed.load()
            ) {
                return;
            }


            processRequest(
                connection,
                request
            );
        }
    );
}


// ============================================================
// IOCP WORKER
// ============================================================

void iocpWorker(
    HANDLE iocp,
    ThreadPool& workers
) {

    while (true) {

        DWORD bytesTransferred = 0;

        ULONG_PTR completionKey = 0;

        OVERLAPPED* overlapped = nullptr;


        BOOL result =
            GetQueuedCompletionStatus(
                iocp,
                &bytesTransferred,
                &completionKey,
                &overlapped,
                INFINITE
            );


        Connection* connection =
            reinterpret_cast<
                Connection*
            >(completionKey);


        if (
            connection == nullptr
        ) {

            continue;
        }


        if (
            !result
        ) {

            closeConnection(
                connection
            );

            continue;
        }


        // ====================================================
        // RECEIVE COMPLETION
        // ====================================================

        if (
            overlapped ==
            &connection->receiveOverlapped
        ) {

            handleReceiveCompletion(
                connection,
                bytesTransferred,
                workers
            );

            continue;
        }


        // ====================================================
        // SEND COMPLETION
        // ====================================================

        if (
            overlapped ==
            &connection->sendOverlapped
        ) {

            connection->sendPending.store(
                false
            );


            if (
                connection->closed.load()
            ) {

                continue;
            }


            // Clear response.
            connection->sendBuffer.clear();


            // Ready for next request.
            startReceive(
                connection
            );


            continue;
        }
    }
}


// ============================================================
// MAIN
// ============================================================

int main() {

    // ========================================================
    // WINSOCK
    // ========================================================

    WSADATA wsaData;


    if (
        WSAStartup(
            MAKEWORD(2, 2),
            &wsaData
        ) != 0
    ) {

        std::cerr
            << "WSAStartup failed\n";

        return 1;
    }


    // ========================================================
    // CREATE LISTEN SOCKET
    // ========================================================

    SOCKET serverSocket =
        socket(
            AF_INET,
            SOCK_STREAM,
            IPPROTO_TCP
        );


    if (
        serverSocket ==
        INVALID_SOCKET
    ) {

        std::cerr
            << "Socket creation failed\n";

        WSACleanup();

        return 1;
    }


    // ========================================================
    // REUSE ADDRESS
    // ========================================================

    BOOL reuse = TRUE;


    setsockopt(
        serverSocket,
        SOL_SOCKET,
        SO_REUSEADDR,
        reinterpret_cast<char*>(&reuse),
        sizeof(reuse)
    );


    // ========================================================
    // SERVER ADDRESS
    // ========================================================

    sockaddr_in serverAddress{};


    serverAddress.sin_family =
        AF_INET;


    serverAddress.sin_addr.s_addr =
        INADDR_ANY;


    serverAddress.sin_port =
        htons(PORT);


    // ========================================================
    // BIND
    // ========================================================

    if (
        bind(
            serverSocket,
            reinterpret_cast<
                sockaddr*
            >(&serverAddress),
            sizeof(serverAddress)
        ) == SOCKET_ERROR
    ) {

        std::cerr
            << "Bind failed\n";

        closesocket(
            serverSocket
        );

        WSACleanup();

        return 1;
    }


    // ========================================================
    // LISTEN
    // ========================================================

    if (
        listen(
            serverSocket,
            LISTEN_BACKLOG
        ) == SOCKET_ERROR
    ) {

        std::cerr
            << "Listen failed\n";

        closesocket(
            serverSocket
        );

        WSACleanup();

        return 1;
    }


    // ========================================================
    // CREATE IOCP
    // ========================================================

    HANDLE iocp =
        CreateIoCompletionPort(
            INVALID_HANDLE_VALUE,
            nullptr,
            0,
            IOCP_THREADS
        );


    if (
        iocp == nullptr
    ) {

        std::cerr
            << "IOCP creation failed\n";

        closesocket(
            serverSocket
        );

        WSACleanup();

        return 1;
    }


    // ========================================================
    // APPLICATION WORKERS
    // ========================================================

    ThreadPool workers(
        WORKER_THREADS
    );


    // ========================================================
    // IOCP THREADS
    // ========================================================

    std::vector<std::thread>
        iocpThreads;


    for (
        int i = 0;
        i < IOCP_THREADS;
        ++i
    ) {

        iocpThreads.emplace_back(
            [&iocp, &workers]() {

                iocpWorker(
                    iocp,
                    workers
                );
            }
        );
    }


    // ========================================================
    // SERVER INFO
    // ========================================================

    std::cout
        << "=====================================\n"
        << " Movie Ticket Booking Server\n"
        << "=====================================\n"
        << "Port           : "
        << PORT
        << "\n"
        << "IOCP Threads   : "
        << IOCP_THREADS
        << "\n"
        << "Worker Threads : "
        << WORKER_THREADS
        << "\n"
        << "Architecture   : Windows IOCP\n"
        << "                 + Thread Pool\n"
        << "Connection     : HTTP Keep-Alive\n"
        << "=====================================\n";


    // ========================================================
    // ACCEPT LOOP
    // ========================================================

    while (true) {

        SOCKET clientSocket =
            accept(
                serverSocket,
                nullptr,
                nullptr
            );


        if (
            clientSocket ==
            INVALID_SOCKET
        ) {

            std::cerr
                << "Accept failed\n";

            continue;
        }


        // ----------------------------------------------------
        // Create connection state.
        //
        // Intentionally retained for the lifetime of the
        // process. With 1000 concurrent VUs this is tiny
        // memory overhead and avoids unsafe deletion while
        // Windows still has an overlapped operation pending.
        // ----------------------------------------------------

        Connection* connection =
            new Connection(
                clientSocket,
                iocp
            );


        // ----------------------------------------------------
        // Associate socket with IOCP.
        // ----------------------------------------------------

        HANDLE associated =
            CreateIoCompletionPort(
                reinterpret_cast<HANDLE>(
                    clientSocket
                ),
                iocp,
                reinterpret_cast<ULONG_PTR>(
                    connection
                ),
                0
            );


        if (
            associated == nullptr
        ) {

            std::cerr
                << "Failed to associate socket with IOCP\n";

            closeConnection(
                connection
            );

            delete connection;

            continue;
        }


        // ----------------------------------------------------
        // Start asynchronous receive.
        // ----------------------------------------------------

        startReceive(
            connection
        );
    }


    // ========================================================
    // SHUTDOWN
    // ========================================================

    closesocket(
        serverSocket
    );


    CloseHandle(
        iocp
    );


    for (
        auto& thread :
        iocpThreads
    ) {

        if (
            thread.joinable()
        ) {

            thread.join();
        }
    }


    WSACleanup();


    return 0;
}