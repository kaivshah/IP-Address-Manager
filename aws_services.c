#include "aws_services.h"
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/PutItemRequest.h>
#include <aws/dynamodb/model/AttributeValue.h>
#include <fstream>
#include <iostream>
#include <memory>

void initializeAWS() {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    atexit([]() { Aws::ShutdownAPI(options); }); // Ensures proper shutdown
}

void uploadErrorLogToS3(const char *bucket, const char *filePath) {
    Aws::S3::S3Client s3_client;
    Aws::S3::Model::PutObjectRequest request;

    request.SetBucket(bucket);
    request.SetKey("error-log.txt");

    // Open the file for reading
    std::shared_ptr<Aws::IOStream> input_data = Aws::MakeShared<Aws::FStream>(
        "ErrorLogStream", filePath, std::ios_base::in | std::ios_base::binary);
    
    if (!input_data->good()) {
        std::cerr << "Failed to open file " << filePath << " for reading." << std::endl;
        return;
    }

    request.SetBody(input_data);

    auto outcome = s3_client.PutObject(request);

    if (outcome.IsSuccess()) {
        std::cout << "Error log uploaded to S3 bucket " << bucket << " successfully!" << std::endl;
    } else {
        std::cerr << "Failed to upload error log: " << outcome.GetError().GetMessage() << std::endl;
    }
}

void displayErrorLog(const char *bucket) {
    Aws::S3::S3Client s3_client;
    Aws::S3::Model::GetObjectRequest request;

    request.SetBucket(bucket);
    request.SetKey("error-log.txt");

    auto outcome = s3_client.GetObject(request);

    if (outcome.IsSuccess()) {
        std::cout << "Error log contents:\n";
        auto &retrieved_file = outcome.GetResult().GetBody();
        std::cout << retrieved_file.rdbuf() << std::endl;
    } else {
        std::cerr << "Failed to retrieve error log: " << outcome.GetError().GetMessage() << std::endl;
    }
}

void addEntryToDynamoDB(const char *ip, const char *alias) {
    Aws::DynamoDB::DynamoDBClient dynamo_client;

    Aws::DynamoDB::Model::PutItemRequest request;
    request.SetTableName("IPAliases");

    // Set IP as the primary key
    Aws::DynamoDB::Model::AttributeValue ip_attr;
    ip_attr.SetS(ip);
    request.AddItem("IP", ip_attr);

    // Set Alias as another attribute
    Aws::DynamoDB::Model::AttributeValue alias_attr;
    alias_attr.SetS(alias);
    request.AddItem("Alias", alias_attr);

    auto outcome = dynamo_client.PutItem(request);

    if (outcome.IsSuccess()) {
        std::cout << "Entry added to DynamoDB successfully: IP=" << ip << ", Alias=" << alias << std::endl;
    } else {
        std::cerr << "Failed to add entry to DynamoDB: " << outcome.GetError().GetMessage() << std::endl;
    }
}
