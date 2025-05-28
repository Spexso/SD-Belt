package gtu.cse.cse396.sdbelt.system.infra.controller;

import java.util.List;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PatchMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import lombok.RequiredArgsConstructor;
import gtu.cse.cse396.sdbelt.system.domain.model.BeltDirection;
import gtu.cse.cse396.sdbelt.system.domain.model.System;
import gtu.cse.cse396.sdbelt.system.domain.model.SystemLatestInfo;
import gtu.cse.cse396.sdbelt.system.domain.service.SystemService;
import gtu.cse.cse396.sdbelt.system.infra.adapter.SystemStatusInfo;
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
    @PostMapping("/system/start")
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
    @PostMapping("/system/stop")
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
    @PostMapping("/system/restart")
    public Response<String> restartSystem() {
        service.restart();
        return ResponseBuilder.build(200, "System restarted successfully");
    }

    @PostMapping("/system/logs")
    public Response<String> saveInfo(@RequestBody SystemLatestInfo info) {
        java.lang.System.out.println("Saving system info: " + info);
        service.saveStatus(info);
        return ResponseBuilder.build(200, "System info saved successfully");
    }

    @GetMapping("/system/logs")
    public Response<List<SystemLatestInfo>> getLogs() {
        List<SystemLatestInfo> logs = service.getLogs();
        return ResponseBuilder.build(200, logs);
    }

    @Operation(summary = "Restart the belt system", description = "Restart the belt system")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "System restarted successfully"),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to restart system.")
    })
    @PostMapping("/system/shutdown")
    public Response<String> shutdownSystem() {
        service.shutdown();
        return ResponseBuilder.build(200, "System shutdown successfully");
    }

    @Operation(summary = "Restart the belt system", description = "Restart the belt system")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "System restarted successfully"),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to restart system.")
    })
    @PostMapping("/system/reverse")
    public Response<String> reverse() {
        java.lang.System.out.println("Reversing the system...");
        service.reverse();
        return ResponseBuilder.build(200, "System reversed successfully");
    }

    @Operation(summary = "Update system info", description = "Update system info")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "System info updated successfully"),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to update system info.")
    })
    @PostMapping("/system/update")
    public Response<String> updateSystem(@RequestParam String name, @RequestParam String description,
            @RequestParam Integer speed, @RequestParam Double threshold, @RequestParam String beltDirection) {
        service.update(name, description, speed, threshold,
                BeltDirection.valueOf(beltDirection.toUpperCase()));
        return ResponseBuilder.build(200, "System info updated successfully");
    }

    @PostMapping("/system/threshold")
    public Response<String> updateSystemThreshold(@RequestBody String body) {
        try {
            double threshold = Double.parseDouble(body.trim());
            service.updateThreshold(threshold);
            return ResponseBuilder.build(200, "System threshold updated successfully");
        } catch (NumberFormatException e) {
            return ResponseBuilder.build(400, "Invalid threshold value: must be an double");
        }
    }

    @PostMapping("/system/speed")
    public Response<String> updateSystemSpeed(@RequestBody String body) {
        try {
            int speed = Integer.parseInt(body.trim());
            service.updateSpeed(speed);
            return ResponseBuilder.build(200, "System speed updated successfully");
        } catch (NumberFormatException e) {
            return ResponseBuilder.build(400, "Invalid speed value: must be an integer");
        }
    }

    @Operation(summary = "Update system info", description = "Update system info")
    @ApiResponses(value = {
            @ApiResponse(responseCode = "200", description = "System info updated successfully"),
            @ApiResponse(responseCode = "401", description = "Unauthorized request. Please check your credentials."),
            @ApiResponse(responseCode = "500", description = "Internal server error. Unable to update system info.")
    })
    @PostMapping("/system/info")
    public Response<String> infoUpdate(@RequestBody SystemStatusInfo info) {
        service.updateInfo(info);
        return ResponseBuilder.build(200, "System info updated successfully");
    }
}
