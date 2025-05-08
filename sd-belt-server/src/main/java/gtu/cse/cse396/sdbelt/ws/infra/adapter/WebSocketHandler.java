package gtu.cse.cse396.sdbelt.ws.infra.adapter;

import org.springframework.stereotype.Component;
import org.springframework.web.socket.CloseStatus;
import org.springframework.web.socket.TextMessage;
import org.springframework.web.socket.WebSocketSession;
import org.springframework.web.socket.handler.TextWebSocketHandler;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import gtu.cse.cse396.sdbelt.ws.domain.model.Event;
import gtu.cse.cse396.sdbelt.ws.domain.model.EventHandler;
import gtu.cse.cse396.sdbelt.ws.domain.model.RawEvent;
import gtu.cse.cse396.sdbelt.ws.domain.registry.EventHandlerRegistry;
import gtu.cse.cse396.sdbelt.ws.domain.service.WebSocketService;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import java.io.IOException;

@Slf4j
@RequiredArgsConstructor
@Component
public class WebSocketHandler extends TextWebSocketHandler {
    
    private final EventHandlerRegistry eventHandlerRegistry;
    private final ObjectMapper objectMapper;
    private final WebSocketService webSocketService;
        
    @Override
    public void afterConnectionEstablished(WebSocketSession session) {
        log.info("Connection established with client: " + session.getId());
        webSocketService.registerSession(session);
    }
    
    @Override
    protected void handleTextMessage(WebSocketSession session, TextMessage message) throws IOException {
        try {
            String payload = message.getPayload();
            log.info("Received message from client {}: {}", session.getId(), payload);
            
            RawEvent raw = objectMapper.readValue(payload, RawEvent.class);
            Class<? extends Event<?>> eventClass = eventHandlerRegistry.getDataType(raw.type());
            EventHandler<?> handler = eventHandlerRegistry.getHandler(raw.type());
            
            if (eventClass != null && handler != null) {
                // Parse the payload directly into the specific event class
                Event<?> event = objectMapper.readValue(payload, eventClass);
                dispatchEvent(event, handler);
            } else {
                log.warn("No handler or class found for event type: {}", raw.type());
            }
            
            // Echo the message back
            session.sendMessage(new TextMessage("Server received: " + payload));
        } catch (Exception e) {
            log.error("Error processing message: {}", e.getMessage(), e);
        }
    }

    @SuppressWarnings("unchecked")
    private <T> void dispatchEvent(Event<?> event, EventHandler<?> handler) {
        // Safe cast since the registry ensures handler and event type match
        ((EventHandler<T>)handler).handle((Event<T>)event);
    }
    
    @Override
    public void afterConnectionClosed(WebSocketSession session, CloseStatus status) {
        log.info("Connection closed with client: " + session.getId() + ", status: " + status);
        webSocketService.removeSession(session);
    }
    
    @Override
    public void handleTransportError(WebSocketSession session, Throwable exception) {
        log.error("Transport error with client {}: {}", session.getId(), exception.getMessage(), exception);
    }
}