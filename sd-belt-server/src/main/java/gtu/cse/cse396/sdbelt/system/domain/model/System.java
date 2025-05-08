package gtu.cse.cse396.sdbelt.system.domain.model;

import java.time.LocalDateTime;
import java.util.UUID;

import lombok.Builder;

@Builder
public record System(

    /**
     * Unique identifier for the system.
     * <p>
     * This field is typically a UUID that uniquely identifies the system in the database.
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
     * This field provides additional context or details about the system's purpose or functionality.
     */
    String description,

    /**
     * Timestamp indicating when the system was created.
     * <p>
     * This field is automatically set when the system is first created and is not meant to be modified.
     */
    LocalDateTime createdAt,

    /**
     * Timestamp indicating when the system was last updated.
     * <p>
     * This field is automatically updated whenever the system's details are modified.
     */
    LocalDateTime runAt,

    /**
     * Status of the system.
     * <p>
     * This field indicates whether the system is currently active, inactive, or in a failed state.
     */
    SystemStatus status,

    /**
     * Accuracy of the system.
     * <p>
     * This field indicates the precision of the system's operations, typically represented as a percentage.
     */
    Integer accuracy,

    /**
     * Speed of the system.
     * <p>
     * This field indicates the operational speed of the system, typically measured in units per minute.
     */
    Integer speed
) {}
