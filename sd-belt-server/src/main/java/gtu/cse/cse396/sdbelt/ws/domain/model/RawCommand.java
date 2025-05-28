package gtu.cse.cse396.sdbelt.ws.domain.model;

import com.fasterxml.jackson.databind.JsonNode;

public record RawCommand(
                CommandType type,
                JsonNode data) {

        public static final <T> RawCommand of(Command<T> command) {
                return new RawCommand(
                                command.type(),
                                command.getContent());
        }
}
