plugins {
    application
    id("java")
}

application {
    mainClass = "org.example.Main"
}

tasks.withType<JavaExec> {
    jvmArgs = listOf("-Xss8m")
}

group = "org.example"
version = "1.0-SNAPSHOT"

repositories {
    mavenCentral()
}

dependencies {
    compileOnly("org.jetbrains:annotations:24.0.1")
    testImplementation(platform("org.junit:junit-bom:5.10.0"))
    testImplementation("org.junit.jupiter:junit-jupiter")
}

tasks.test {
    useJUnitPlatform()
}
