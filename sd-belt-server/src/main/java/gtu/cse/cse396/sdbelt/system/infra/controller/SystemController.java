package gtu.cse.cse396.sdbelt.system.infra.controller;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PatchMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.parameters.RequestBody;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import lombok.RequiredArgsConstructor;
import gtu.cse.cse396.sdbelt.system.domain.model.System;
import gtu.cse.cse396.sdbelt.system.domain.service.SystemService;
import gtu.cse.cse396.sdbelt.shared.model.Response;
import gtu.cse.cse396.sdbelt.shared.model.ResponseBuilder;

@RestController
@RequiredArgsConstructor
@RequestMapping("/api/v1")
public class SystemController {
    
    private final SystemService service;

    @Operation(summary = "Get system info", description = "Get system info")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "System info retrieved successfully"),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to retrieve system info.")
    })
    @GetMapping("/system/info")
    public Response<System> getSystemInfo() {
        System system = service.get();
        return ResponseBuilder.build(200, system);
    }

    @Operation(summary = "Start the belt system", description = "Start the belt system")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "System started successfully"),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to start system.")
    })
    @PatchMapping("/system/start")
    public Response<String> startSystem() {
        service.start();
        return ResponseBuilder.build(200, "System started successfully");
    }

    @Operation(summary = "Stop the belt system", description = "Stop the belt system")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "System stopped successfully"),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to stop system.")
    })
    @PatchMapping("/system/stop")
    public Response<String> stopSystem() {
        service.stop();
        return ResponseBuilder.build(200, "System stopped successfully");
    }

    @Operation(summary = "Restart the belt system", description = "Restart the belt system")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "System restarted successfully"),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to restart system.")
    })
    @PatchMapping("/system/restart")
    public Response<String> restartSystem() {
        service.restart();
        return ResponseBuilder.build(200, "System restarted successfully");
    }

    @Operation(summary = "Update system info", description = "Update system info")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "System info updated successfully"),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to update system info.")
    })
    @PostMapping("/system/update")
    public Response<String> updateSystem(@RequestBody String name, @RequestBody String description) {
        service.update(name, description);
        return ResponseBuilder.build(200, "System info updated successfully");
    }
}
