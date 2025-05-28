package gtu.cse.cse396.sdbelt.ws.domain.model;

import com.fasterxml.jackson.databind.JsonNode;

public record RawMessage(
        Long id,
        MessageType type,
        JsonNode content,
        long timestamp) {

    public static final <T> RawMessage ofCommand(Command<T> command) {
        return new RawMessage(
                generateId(),
                MessageType.COMMAND,
                command.getContent(),
                System.currentTimeMillis());
    }

    public static final <T> RawMessage ofEvent(Event<T> event) {
        return new RawMessage(
                generateId(),
                MessageType.EVENT,
                event.getContent(),
                System.currentTimeMillis());
    }

    private static Long generateId() {
        return System.currentTimeMillis();
    }

    public final String serialize() {
        return String.format("{\"id\":%d,\"type\":\"%s\",\"content\":%s,\"timestamp\":%d}",
                id, type.getType(), content.toString(), timestamp);
    }
}