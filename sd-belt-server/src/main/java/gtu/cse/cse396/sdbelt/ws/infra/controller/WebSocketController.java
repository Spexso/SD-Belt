package gtu.cse.cse396.sdbelt.ws.infra.controller;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import lombok.RequiredArgsConstructor;
import gtu.cse.cse396.sdbelt.shared.model.Response;
import gtu.cse.cse396.sdbelt.shared.model.ResponseBuilder;
import gtu.cse.cse396.sdbelt.ws.domain.model.MessageType;
import gtu.cse.cse396.sdbelt.ws.domain.model.RawMessage;
import gtu.cse.cse396.sdbelt.ws.domain.service.WebSocketService;

@RestController
@RequiredArgsConstructor
@RequestMapping("/api/v1/ws")
public class WebSocketController {

    private final WebSocketService service;

    @GetMapping(params = "send")
    public Response<Boolean> trySend(@RequestParam String send) {
        RawMessage message = new RawMessage(32L, MessageType.COMMAND, null, System.currentTimeMillis());
        boolean success = service.send(message);
        return ResponseBuilder.build(200, success);
    }
}
