package gtu.cse.cse396.sdbelt.ws.domain.model;

import com.fasterxml.jackson.databind.JsonNode;

import io.swagger.v3.core.util.Json;

public record ReverseBeltCommand(
        Void data,
        long timestamp) implements Command<Void> {

    public ReverseBeltCommand() {
        this(null, System.currentTimeMillis());
    }

    @Override
    public CommandType type() {
        return CommandType.REVERSE_BELT_COMMAND;
    }

    @Override
    public JsonNode getContent() {
        return Json.mapper().convertValue(data, JsonNode.class);
    }
}
