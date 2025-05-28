package gtu.cse.cse396.sdbelt.system.infra.adapter;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpEntity;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Component;
import org.springframework.web.client.RestTemplate;

import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;

/**
 * Abstracted command sender to handle HTTP REST communication
 * Mirrors the Qt NetworkManager POST requests functionality
 */
@Component
@RequiredArgsConstructor
@Slf4j
class SystemCommandSender {

    private final RestTemplate restTemplate;

    @Value("${system.server.url}")
    private String serverUrl;

    /**
     * Send start command via HTTP POST
     */
    public void sendSpeedCommand(int speedPercent) {
        try {
            String url = serverUrl + "/speed";
            HttpEntity<String> request = createTextPlainRequest(String.valueOf(speedPercent));

            ResponseEntity<String> response = restTemplate.postForEntity(url, request, String.class);
            log.debug("[POST /speed] Sent: {} | Response: {}", speedPercent, response.getBody());
        } catch (Exception e) {
            log.error("Failed to send speed command", e);
            throw new RuntimeException("Speed command failed", e);
        }
    }

    public void sendThresholdCommand(double threshold) {
        try {
            String url = serverUrl + "/threshold";
            HttpEntity<String> request = createTextPlainRequest(String.valueOf(threshold));

            ResponseEntity<String> response = restTemplate.postForEntity(url, request, String.class);
            log.debug("[POST /threshold] Sent: {} | Response: {}", threshold, response.getBody());
        } catch (Exception e) {
            log.debug("Failed to send speed command");
            throw new RuntimeException("Thresh command failed", e);
        }
    }

    public void sendReverseCommand() {
        try {
            String url = serverUrl + "/rev";
            HttpEntity<String> request = createTextPlainRequest("REV");

            ResponseEntity<String> response = restTemplate.postForEntity(url, request, String.class);
            log.debug("[POST /rev] Sent: REV | Response: {}", response.getBody());
        } catch (Exception e) {
            log.error("Failed to send reverse command", e);
            throw new RuntimeException("Reverse command failed", e);
        }
    }

    /**
     * Send shutdown command via HTTP POST
     */
    public void sendShutdownCommand() {
        try {
            String url = serverUrl + "/shutdown";
            HttpEntity<String> request = createTextPlainRequest("SHUTDOWN");

            ResponseEntity<String> response = restTemplate.postForEntity(url, request, String.class);
            log.debug("[POST /shutdown] Response: {}", response.getBody());
        } catch (Exception e) {
            log.error("Failed to send shutdown command", e);
            throw new RuntimeException("Shutdown command failed", e);
        }
    }

    /**
     * Send stop command via HTTP POST
     */
    public void sendStopCommand() {
        try {
            String url = serverUrl + "/stop";
            HttpEntity<String> request = createTextPlainRequest("STOP");

            ResponseEntity<String> response = restTemplate.postForEntity(url, request, String.class);
            log.debug("[POST /stop] Response: {}", response.getBody());
        } catch (Exception e) {
            log.error("Failed to send stop command", e);
            throw new RuntimeException("Stop command failed", e);
        }
    }

    /**
     * Send restart command via HTTP POST
     */
    public void sendRestartCommand() {
        try {
            String url = serverUrl + "/restart";
            HttpEntity<String> request = createTextPlainRequest("RESTART");

            ResponseEntity<String> response = restTemplate.postForEntity(url, request, String.class);
            log.debug("[POST /restart] Response: {}", response.getBody());
        } catch (Exception e) {
            log.error("Failed to send restart command", e);
            throw new RuntimeException("Restart command failed", e);
        }
    }

    /**
     * Send emergency stop command via HTTP POST
     * Equivalent to Qt OnEmergencyStopClicked() POST request
     */
    public void sendEmergencyStopCommand() {
        try {
            String url = serverUrl + "/stop";
            HttpEntity<String> request = createTextPlainRequest("STOP\n");

            ResponseEntity<String> response = restTemplate.postForEntity(url, request, String.class);
            log.warn("[POST /stop] Emergency Stop Response: {}", response.getBody());
        } catch (Exception e) {
            log.error("Failed to send emergency stop command", e);
            throw new RuntimeException("Emergency stop command failed", e);
        }
    }

    /**
     * Send speed adjustment command via HTTP POST
     * Equivalent to Qt OnSpeedAdjusted() POST request
     */
    public void sendSpeedAdjustCommand(int speed) {
        try {
            String url = serverUrl + "/speed";
            HttpEntity<String> request = createTextPlainRequest(String.valueOf(speed));

            ResponseEntity<String> response = restTemplate.postForEntity(url, request, String.class);
            log.debug("[POST /speed] Speed Adjust Response: {}", response.getBody());
        } catch (Exception e) {
            log.error("Failed to send speed adjust command for speed: {}", speed, e);
            throw new RuntimeException("Speed adjust command failed", e);
        }
    }

    /**
     * Create HTTP request with text/plain content type
     * Mirrors the Qt QNetworkRequest setup
     */
    private HttpEntity<String> createTextPlainRequest(String body) {
        org.springframework.http.HttpHeaders headers = new org.springframework.http.HttpHeaders();
        headers.setContentType(MediaType.TEXT_PLAIN);
        return new HttpEntity<>(body, headers);
    }
}