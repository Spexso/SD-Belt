package gtu.cse.cse396.sdbelt.product.infra.adapter;

import gtu.cse.cse396.sdbelt.product.domain.service.ProductService;
import gtu.cse.cse396.sdbelt.product.domain.model.Product;
import lombok.RequiredArgsConstructor;
import org.springframework.boot.ApplicationArguments;
import org.springframework.boot.ApplicationRunner;
import org.springframework.stereotype.Component;

import java.util.List;

@Component
@RequiredArgsConstructor
public class ProductDataInitializer implements ApplicationRunner {

    static final String[] DEFAULT_PRODUCT_IDS = { "BANANA", "APPLE", "PORTAKAL" };

    static final String[] DEFAULT_PRODUCT_NAMES = {
            "Muz", "Elma", "Portakal"
    };
    static final String[] DEFAULT_PRODUCT_DESCRIPTIONS = {
            "Muz", "Elma", "Portakal"
    };

    static final String[] DEFAULT_PRODUCT_IMAGE_IDS = {
            "apple.png", "plastic-bottle.png", "pencil.png", "eraser.png", "box.png"
    };

    int DEFAULT_PRODUCT_COUNT = DEFAULT_PRODUCT_IDS.length;
    private final ProductService productService;

    @Override
    public void run(ApplicationArguments args) {
        List<Product> existingProducts = productService.list();

        if (existingProducts.size() >= DEFAULT_PRODUCT_COUNT) {
            System.out.println("Database already initialized with at least 5 products.");
            return;
        }

        for (int i = 0; i < DEFAULT_PRODUCT_COUNT - existingProducts.size(); i++) {
            String productId = DEFAULT_PRODUCT_IDS[i % DEFAULT_PRODUCT_IDS.length];
            String name = DEFAULT_PRODUCT_NAMES[i % DEFAULT_PRODUCT_NAMES.length];
            String description = DEFAULT_PRODUCT_DESCRIPTIONS[i % DEFAULT_PRODUCT_DESCRIPTIONS.length];
            String imageId = DEFAULT_PRODUCT_IMAGE_IDS[i % DEFAULT_PRODUCT_IMAGE_IDS.length];
            productService.create(productId, name, description, imageId);
            System.out.println("Inserted product: " + name);
        }

        System.out.println("Product initialization completed.");
    }
}
