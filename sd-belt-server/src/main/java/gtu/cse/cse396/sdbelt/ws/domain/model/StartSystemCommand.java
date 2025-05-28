package gtu.cse.cse396.sdbelt.ws.domain.model;

import com.fasterxml.jackson.databind.JsonNode;

import io.swagger.v3.core.util.Json;

public record StartSystemCommand(
        Void data,
        long timestamp) implements Command<Void> {

    public StartSystemCommand() {
        this(null, System.currentTimeMillis());
    }

    @Override
    public CommandType type() {
        return CommandType.START_SYSTEM_COMMAND;
    }

    @Override
    public JsonNode getContent() {
        return Json.mapper().convertValue(data, JsonNode.class);
    }
}
