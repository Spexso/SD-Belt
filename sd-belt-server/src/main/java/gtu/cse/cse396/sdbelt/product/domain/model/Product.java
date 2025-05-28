package gtu.cse.cse396.sdbelt.product.domain.model;

import java.time.LocalDateTime;
import java.util.UUID;

import lombok.Builder;

/**
 * Represents a product that moves through the conveyor belt and may be scanned.
 * <p>
 * Contains identifying and descriptive information, as well as creation and
 * update timestamps.
 */
@Builder
public record Product(

                /**
                 * The unique identifier of the product, typically represented as a string UUID.
                 * <p>
                 * This ID is used to reference the product in scans and across other services.
                 */
                String id,

                /**
                 * The human-readable name of the product.
                 * <p>
                 * Used for UI display and product identification.
                 */
                String name,

                /**
                 * A brief textual description of the product.
                 * <p>
                 * Can be used to provide additional context or classification details.
                 */
                String description,

                /**
                 * The timestamp when the product was initially created in the system.
                 * <p>
                 * This value is typically set automatically when the product is first
                 * persisted.
                 */
                LocalDateTime createdAt,

                /**
                 * The timestamp when the product was last updated.
                 * <p>
                 * Automatically updated whenever the product's name or description is modified.
                 */
                LocalDateTime updatedAt,

                String imageId) {
}
