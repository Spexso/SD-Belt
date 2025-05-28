package gtu.cse.cse396.sdbelt.ws.infra.adapter;

import org.springframework.stereotype.Service;
import org.springframework.web.socket.TextMessage;
import org.springframework.web.socket.WebSocketSession;

import com.fasterxml.jackson.databind.ObjectMapper;

import gtu.cse.cse396.sdbelt.ws.domain.model.RawMessage;
import gtu.cse.cse396.sdbelt.ws.domain.service.WebSocketService;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import java.io.IOException;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

@Service
@Slf4j
@RequiredArgsConstructor
public class WebSocketAdapter implements WebSocketService {

    private static final ConcurrentHashMap<String, WebSocketSession> sessions = new ConcurrentHashMap<>();
    private final ObjectMapper objectMapper;

    /**
     * Register a WebSocket session
     */
    @Override
    public void registerSession(WebSocketSession session) {
        sessions.put(session.getId(), session);
        log.info("Session registered: {}", session.getId());
    }

    /**
     * Remove a WebSocket session
     */
    @Override
    public void removeSession(WebSocketSession session) {
        sessions.remove(session.getId());
        log.info("Session removed: {}", session.getId());
    }

    /**
     * Send a message to a specific client by session ID
     */
    @Override
    public boolean send(RawMessage message) {
        for (Map.Entry<String, WebSocketSession> entry : sessions.entrySet()) {
            WebSocketSession session = entry.getValue();
            if (trySend(session, message.serialize())) {
                return true;
            }
        }
        return false;
    }

    private boolean trySend(WebSocketSession session, Object message) {
        if (session != null && session.isOpen()) {
            try {
                String jsonMessage = objectMapper.writeValueAsString(message);
                session.sendMessage(new TextMessage(jsonMessage));
                return true;
            } catch (IOException e) {
                log.error("Error sending message to client {}: {}", session.getId(), e.getMessage(), e);
            }
        } else {
            log.warn("Client session {} not found or closed", session.getId());
        }
        return false;
    }

    /**
     * Get all active session IDs
     */
    @Override
    public Map<String, WebSocketSession> getSessions() {
        return sessions;
    }

    /**
     * Get active session count
     */
    @Override
    public int getActiveSessionCount() {
        return (int) sessions.values().stream()
                .filter(WebSocketSession::isOpen)
                .count();
    }
}
