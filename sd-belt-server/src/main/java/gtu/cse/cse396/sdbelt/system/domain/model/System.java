package gtu.cse.cse396.sdbelt.system.domain.model;

import java.time.LocalDateTime;
import java.util.UUID;

import lombok.Builder;

@Builder
public record System(

        /**
         * Unique identifier for the system.
         * <p>
         * This field is typically a UUID that uniquely identifies the system in the
         * database.
         */
        UUID id,

        /**
         * Name of the system.
         * <p>
         * This field is used to identify the system in user interfaces and reports.
         */
        String name,

        /**
         * Description of the system.
         * <p>
         * This field provides additional context or details about the system's purpose
         * or functionality.
         */
        String description,

        /**
         * Timestamp indicating when the system was created.
         * <p>
         * This field is automatically set when the system is first created and is not
         * meant to be modified.
         */
        LocalDateTime createdAt,

        /**
         * Timestamp indicating when the system was last updated.
         * <p>
         * This field is automatically updated whenever the system's details are
         * modified.
         */
        LocalDateTime runAt,

        /**
         * Status of the system.
         * <p>
         * This field indicates whether the system is currently active, inactive, or in
         * a failed state.
         */
        SystemStatus status,

        /**
         * Accuracy of the system.
         * <p>
         * This field indicates the precision of the system's operations, typically
         * represented as a percentage.
         */
        Double threshold,

        /**
         * Speed of the system.
         * <p>
         * This field indicates the operational speed of the system, typically measured
         * in units per minute.
         */
        Integer speed,

        BeltDirection beltDirection,

        Double cpuUsage,
        Double cpuTemperature,
        Double memoryUsage,

        LocalDateTime lastUpdated) {

    public System copyWith(SystemStatus status, Double cpuUsage, Double cpuTemperature, Double memoryUsage) {
        return System.builder()
                .id(this.id)
                .name(this.name)
                .description(this.description)
                .createdAt(this.createdAt)
                .runAt(this.runAt)
                .status(status)
                .threshold(this.threshold)
                .speed(this.speed)
                .beltDirection(this.beltDirection)
                .cpuUsage(cpuUsage)
                .cpuTemperature(cpuTemperature)
                .memoryUsage(memoryUsage)
                .lastUpdated(LocalDateTime.now())
                .build();
    }

    public System copyWith(SystemStatus status) {
        return System.builder()
                .id(this.id)
                .name(this.name)
                .description(this.description)
                .createdAt(this.createdAt)
                .runAt(this.runAt)
                .status(status)
                .threshold(this.threshold)
                .speed(this.speed)
                .beltDirection(this.beltDirection)
                .cpuUsage(this.cpuUsage)
                .cpuTemperature(this.cpuTemperature)
                .memoryUsage(this.memoryUsage)
                .lastUpdated(LocalDateTime.now())
                .build();
    }

    public System copyWith(String name, String description) {
        return System.builder()
                .id(this.id)
                .name(name)
                .description(description)
                .createdAt(this.createdAt)
                .runAt(this.runAt)
                .status(this.status)
                .threshold(this.threshold)
                .speed(this.speed)
                .beltDirection(this.beltDirection)
                .cpuUsage(this.cpuUsage)
                .cpuTemperature(this.cpuTemperature)
                .memoryUsage(this.memoryUsage)
                .lastUpdated(this.lastUpdated)
                .build();
    }

    public System copyWith(String name, String description, SystemStatus status) {
        return System.builder()
                .id(this.id)
                .name(name)
                .description(description)
                .createdAt(this.createdAt)
                .runAt(this.runAt)
                .status(status)
                .threshold(this.threshold)
                .speed(this.speed)
                .beltDirection(this.beltDirection)
                .cpuUsage(this.cpuUsage)
                .cpuTemperature(this.cpuTemperature)
                .memoryUsage(this.memoryUsage)
                .lastUpdated(this.lastUpdated)
                .build();
    }

    public System copyWith(BeltDirection direction) {
        return System.builder()
                .id(id)
                .name(this.name)
                .description(this.description)
                .createdAt(this.createdAt)
                .runAt(this.runAt)
                .status(this.status)
                .threshold(this.threshold)
                .speed(this.speed)
                .beltDirection(direction)
                .cpuUsage(this.cpuUsage)
                .cpuTemperature(this.cpuTemperature)
                .memoryUsage(this.memoryUsage)
                .lastUpdated(this.lastUpdated)
                .build();
    }

    public System copyWith(Integer speed, Double threshold) {
        return System.builder()
                .id(this.id)
                .name(this.name)
                .description(this.description)
                .createdAt(this.createdAt)
                .runAt(this.runAt)
                .status(this.status)
                .threshold(threshold)
                .speed(speed)
                .beltDirection(this.beltDirection)
                .cpuUsage(this.cpuUsage)
                .cpuTemperature(this.cpuTemperature)
                .memoryUsage(this.memoryUsage)
                .lastUpdated(this.lastUpdated)
                .build();
    }
}
