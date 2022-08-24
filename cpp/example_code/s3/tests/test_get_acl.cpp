// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX - License - Identifier: Apache - 2.0

 #include <iostream>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/core/utils/UUID.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <fstream>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/DeleteBucketRequest.h>
#include <awsdoc/s3/s3_examples.h>

int main()
{
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        const char* file_name = "my-file.txt";

        Aws::Client::ClientConfiguration config;
        config.region = "us-east-1";

        Aws::S3::S3Client s3_client(config);

        // 1/6. Create the bucket to upload the object to.
        // Create a unique bucket name to increase the chance of success
        // when trying to create the bucket.
        // Format: "my-bucket-" + lowercase UUID.
        Aws::String uuid = Aws::Utils::UUID::RandomUUID();
        Aws::String bucket_name = "my-bucket-" +
            Aws::Utils::StringUtils::ToLower(uuid.c_str());

        Aws::S3::Model::CreateBucketRequest create_bucket_request;
        create_bucket_request.SetBucket(bucket_name);

        Aws::S3::Model::CreateBucketOutcome create_bucket_outcome =
            s3_client.CreateBucket(create_bucket_request);

        if (!create_bucket_outcome.IsSuccess())
        {
            auto err = create_bucket_outcome.GetError();
            std::cout << "Error: GetAcls test setup: Create bucket '" <<
                bucket_name << "': " <<
                err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
            std::cout << "No cleanup needed." << std::endl;

            return 1;
        }

        // 2/6. Create the object to upload, and then upload the object
        // to the bucket.
        // For this test, create a text file named 'my-file.txt' in the same
        // directory as this test.
        std::ofstream myFile(file_name);
        myFile << "My content.";
        myFile.close();

        Aws::S3::Model::PutObjectRequest put_object_request;
        put_object_request.SetBucket(bucket_name);
        put_object_request.SetKey(Aws::String(file_name));

        std::shared_ptr<Aws::IOStream> file_body =
            Aws::MakeShared<Aws::FStream>("SampleAllocationTag", file_name,
                std::ios_base::in | std::ios_base::binary);

        put_object_request.SetBody(file_body);

        Aws::S3::Model::PutObjectOutcome put_object_outcome =
            s3_client.PutObject(put_object_request);

        if (!put_object_outcome.IsSuccess())
        {
            auto err = put_object_outcome.GetError();
            std::cout << "Error: GetAcls test setup: Upload object '" << file_name << "' " <<
                "to bucket '" << bucket_name << "': " <<
                err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
            std::cout << "To clean up, you must delete the bucket '" <<
                bucket_name << "' yourself." << std::endl;

            return 1;
        }

        // 3/6. Get the bucket ACL.
        if (!AwsDoc::S3::GetBucketAcl(bucket_name, config.region))
        {
            std::cout << "Error: GetAcls test: Get ACL for bucket '" <<
                bucket_name << "'. To clean up, you must delete the bucket '" <<
                bucket_name << "' yourself." << std::endl;

            return 1;
        }

        // 4/6. Get the object ACL.
        if (!AwsDoc::S3::GetObjectAcl(bucket_name, file_name, "us-east-1"))
        {
            std::cout << "Error: GetAcls test setup: Get ACL for object '" << file_name << "'. " <<
                "To clean up, you must delete the bucket '" <<
                bucket_name << "' yourself." << std::endl;

            return 1;
        }

        // 5/6. Delete the object from the bucket.
        Aws::S3::Model::DeleteObjectRequest delete_object_request;

        delete_object_request.WithKey(Aws::String(file_name))
            .WithBucket(bucket_name);

        Aws::S3::Model::DeleteObjectOutcome delete_object_outcome =
            s3_client.DeleteObject(delete_object_request);

        if (!delete_object_outcome.IsSuccess())
        {
            auto err = delete_object_outcome.GetError();
            std::cout << "Error: GetAcls test cleanup: Delete object '" << file_name << "' from bucket '" <<
                bucket_name << "':" <<
                err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
            std::cout << "To clean up, you must delete the bucket '" <<
                bucket_name << "' yourself." << std::endl;

            return 1;
        }

        // 6/6. Delete the bucket.
        Aws::S3::Model::DeleteBucketRequest delete_bucket_request;
        delete_bucket_request.SetBucket(bucket_name);

        Aws::S3::Model::DeleteBucketOutcome delete_bucket_outcome =
            s3_client.DeleteBucket(delete_bucket_request);

        if (!delete_bucket_outcome.IsSuccess())
        {
            auto err = delete_bucket_outcome.GetError();
            std::cout << "Error: GetAcls test cleanup: Delete bucket '" <<
                bucket_name << "':" <<
                err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
            std::cout << "To clean up, you must delete the bucket '" <<
                bucket_name << "' yourself." << std::endl;

            return 1;
        }
    }
    Aws::ShutdownAPI(options);

    return 0;
}
