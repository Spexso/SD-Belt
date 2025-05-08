package gtu.cse.cse396.sdbelt.ws.domain.model;

import com.fasterxml.jackson.databind.JsonNode;

public record RawEvent (
    EventType type,
    JsonNode data,
    long timestamp
){}
