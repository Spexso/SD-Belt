package gtu.cse.cse396.sdbelt.ws.domain.service;

import org.springframework.web.socket.WebSocketSession;

import java.util.Map;

public interface WebSocketService {

    /**
     * Register a WebSocket session
     */
    void registerSession(WebSocketSession session);
    
    /**
     * Remove a WebSocket session
     */
    void removeSession(WebSocketSession session);
    
    /**
     * Send a message to a specific client by session ID
     */
    public boolean send(Object message);
    
    /**
     * Get all active session IDs
     */
    Map<String, WebSocketSession> getSessions();
    
    /**
     * Get active session count
     */
    int getActiveSessionCount();
}
