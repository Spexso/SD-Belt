package gtu.cse.cse396.sdbelt.scan.domain.model;

import com.fasterxml.jackson.databind.JsonNode;

import gtu.cse.cse396.sdbelt.ws.domain.model.Event;
import gtu.cse.cse396.sdbelt.ws.domain.model.EventType;
import io.swagger.v3.core.util.Json;

public record ScanEvent(
                Scan data,
                long timestamp) implements Event<Scan> {

        @Override
        public EventType type() {
                return EventType.NEW_SCAN_EVENT;
        }

        @Override
        public JsonNode getContent() {
                return Json.mapper().convertValue(data, JsonNode.class);
        }
}
