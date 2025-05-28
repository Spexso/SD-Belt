package gtu.cse.cse396.sdbelt.system.domain.model;

import java.util.List;

import com.fasterxml.jackson.databind.JsonNode;

import gtu.cse.cse396.sdbelt.ws.domain.model.Event;
import gtu.cse.cse396.sdbelt.ws.domain.model.EventType;
import io.swagger.v3.core.util.Json;

public record InitializeProductsEvent(
                List<ProductInfo> data,
                long timestamp) implements Event<List<ProductInfo>> {

        @Override
        public EventType type() {
                return EventType.NEW_SCAN_EVENT;
        }

        @Override
        public JsonNode getContent() {
                return Json.mapper().convertValue(data, JsonNode.class);
        }
}
