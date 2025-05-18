// udp_sender.hpp
#pragma  once
#include <opencv2/opencv.hpp>
#include <arpa/inet.h>
#include <vector>
#include <cstdint>

struct UdpSender {
    int                 sock  = -1;
    sockaddr_in         dst   {};

    explicit UdpSender(std::string_view ip    = "192.168.1.50",
                       uint16_t        port  = 5000) {
        sock = ::socket(AF_INET, SOCK_DGRAM, 0);
        dst.sin_family = AF_INET;
        dst.sin_port   = htons(port);
        ::inet_pton(AF_INET, ip.data(), &dst.sin_addr);
    }
    void send(int camId, const cv::Mat& img) const {
        std::vector<uchar> buf;
        cv::imencode(".jpg", img, buf, {cv::IMWRITE_JPEG_QUALITY, 80});

        if (buf.size() > 60'000) return;                 // keep it single-datagram

        /* build header */
		std::vector<uint8_t> header(5);
		header[0] = static_cast<uint8_t>(camId);
		uint32_t len_be = htonl(static_cast<uint32_t>(buf.size()));
		std::memcpy(&header[1], &len_be, sizeof(len_be));

		std::array<iovec, 2> iov{
			iovec{header.data(), header.size()},
			iovec{buf.data(), buf.size()}
		};

        
        
        
        msghdr msg{};
        msg.msg_name       = const_cast<sockaddr*>(reinterpret_cast<const sockaddr*>(&dst));
        msg.msg_namelen    = sizeof(dst);
        msg.msg_iov        = iov.data();
        msg.msg_iovlen     = iov.size();
        ::sendmsg(sock, &msg, 0);
    }
};
